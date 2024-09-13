use tokio::{self, io::AsyncReadExt};

use super::handler::StreamHandler;
use super::utils::reorder::RtpPacketReOrder;

pub async fn bind(
    host: &String,
    port: u16,
) -> Result<(tokio::net::UdpSocket, tokio::net::TcpListener), std::io::Error> {
    let local_addr = format!("{host}:{port}",);

    // udp server
    match tokio::net::UdpSocket::bind(&local_addr).await {
        Err(e) => {
            tracing::error!("UdpSocket::bind({}) error, e: {:?}", &local_addr, e);
            return Err(e);
        }
        Ok(udp_socket) => {
            tracing::info!("UdpSocket::bind({}) ok", &local_addr);

            // tcp server
            match tokio::net::TcpListener::bind(&local_addr).await {
                Err(e) => {
                    tracing::error!("TcpListener::bind({}) error, e: {:?}", &local_addr, e);
                    return Err(e);
                }
                Ok(tcp_listener) => {
                    tracing::info!("TcpListener::bind({}) ok", &local_addr);
                    return Ok((udp_socket, tcp_listener));
                }
            }
        }
    }
}

pub async fn run_forever(
    is_running: std::sync::Arc<bool>,
    socket_recv_buffer_size: usize,
    stream_handler: std::sync::Arc<StreamHandler>,
) -> Result<(tokio::task::JoinHandle<()>, tokio::task::JoinHandle<()>), std::io::Error> {
    // udp server
    let udp_is_running = is_running.clone();
    let udp_stream_handler = stream_handler.clone();
    let udp_join_handle = tokio::spawn(async move {
        tracing::info!("udp stream service start, port: {}", udp_stream_handler.port);

        let mut recv_buff = Vec::<u8>::default();
        recv_buff.resize(socket_recv_buffer_size, 0);

        let mut packets_reorder = RtpPacketReOrder::new(5);

        while *udp_is_running {
            match udp_stream_handler
                .stream_udp_socket
                .recv_from(recv_buff.as_mut_slice())
                .await
            {
                Err(e) => {
                    tracing::error!("UdpSocket::recv_from error, e: {:?}", e);
                    return;
                }
                Ok((amount, addr)) => {
                    // dispatch
                    udp_stream_handler.on_rtp(
                        addr,
                        &recv_buff.as_slice()[..amount],
                        &mut packets_reorder,
                    );
                }
            }
        }

        tracing::info!("udp stream service stop, port: {}", udp_stream_handler.port);
    });

    // tcp server
    let tcp_is_running = is_running.clone();
    let tcp_stream_handler = stream_handler.clone();
    let tcp_join_handle = tokio::spawn(async move {
        tracing::info!("tcp stream service start, port: {}", tcp_stream_handler.port);

        let mut packets_reorder = RtpPacketReOrder::new(5);

        match tcp_stream_handler.stream_tcp_listener.accept().await {
            Err(e) => {
                tracing::error!("TcpListener::accept error: {:?}", e);
            }
            Ok((tcp_stream, addr)) => {
                let tcp_stream_mutex_arc = std::sync::Arc::new(tokio::sync::Mutex::new(tcp_stream));

                while *tcp_is_running {
                    // 2 bytes size header
                    let rtp_length = {
                        match tcp_stream_mutex_arc.clone().lock().await.read_u16().await {
                            Err(e) => {
                                tracing::error!("TcpStream::read_u16 error, e: {:?}", e);
                                0
                            }
                            Ok(length) => length,
                        }
                    };
                    if 0 == rtp_length {
                        return;
                    }

                    // content
                    let mut recv_buff = vec![0; rtp_length as usize];
                    match tcp_stream_mutex_arc
                        .clone()
                        .lock()
                        .await
                        .read_exact(&mut recv_buff)
                        .await
                    {
                        Ok(0) => return, // connection closed
                        Err(e) => {
                            tracing::error!("TcpStream::read error, e: {:?}", e);
                            return;
                        }
                        Ok(amount) => {
                            // dispatch
                            tcp_stream_handler.on_rtp(
                                addr,
                                &recv_buff.as_slice()[..amount],
                                &mut packets_reorder,
                            );
                        }
                    };
                }
            }
        }

        tracing::info!("tcp stream service top, port: {}", tcp_stream_handler.port);
    });

    return Ok((udp_join_handle, tcp_join_handle));
}
