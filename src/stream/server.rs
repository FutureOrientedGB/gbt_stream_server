use tokio::{self, io::AsyncReadExt};

use crate::utils::cli::CommandLines;

use super::handler::StreamHandler;

pub async fn bind(
    cli_args: &CommandLines,
) -> Result<(tokio::net::UdpSocket, tokio::net::TcpListener), std::io::Error> {
    let local_addr = format!(
        "{host}:{port}",
        host = cli_args.host,
        port = cli_args.stream_port
    );

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

fn parse_rtp_message(buffer: &[u8]) -> Option<(Vec<u8>, Vec<u8>)> {
    None
}

pub async fn run_forever(
    cli_args: CommandLines,
    stream_handler: std::sync::Arc<StreamHandler>,
) -> Result<(), std::io::Error> {
    // udp server
    let stream_handler_udp = stream_handler.clone();
    let udp_server_handle = tokio::spawn(async move {
        let mut recv_buff = Vec::<u8>::default();
        recv_buff.resize(cli_args.socket_recv_buffer_size, 0);

        loop {
            match stream_handler_udp
                .stream_udp_socket
                .recv_from(recv_buff.as_mut_slice())
                .await
            {
                Err(e) => {
                    tracing::error!("UdpSocket::recv_from error, e: {:?}", e);
                }
                Ok((amount, addr)) => {
                    // dispatch
                    tracing::info!("recv rtp packet from {}, size: {}", addr.to_string(), amount);
                }
            }
        }
    });

    // tcp server
    let tcp_server_handle = tokio::spawn(async move {
        loop {
            match stream_handler.stream_tcp_listener.accept().await {
                Err(e) => {
                    tracing::error!("TcpListener::accept error: {:?}", e);
                }
                Ok((tcp_stream, addr)) => {
                    let stream_handler_cloned = stream_handler.clone();
                    tokio::spawn(async move {
                        let mut buffer = Vec::new();

                        let tcp_stream_mutex_arc =
                            std::sync::Arc::new(tokio::sync::Mutex::new(tcp_stream));

                        loop {
                            let mut recv_buff = vec![0; 1024];
                            let n = match tcp_stream_mutex_arc
                                .clone()
                                .lock()
                                .await
                                .read(&mut recv_buff)
                                .await
                            {
                                Ok(0) => return, // connection closed
                                Ok(n) => n,
                                Err(e) => {
                                    tracing::error!("TcpStream::read error, e: {:?}", e);
                                    return;
                                }
                            };

                            buffer.extend_from_slice(&recv_buff[..n]);

                            while let Some((message, remaining)) = parse_rtp_message(&buffer) {
                                // dispatch
                                tracing::info!("recv rtp packet from {}, size: {}", addr.to_string(), message.len());

                                buffer = remaining;
                            }
                        }
                    });
                }
            }
        }
    });

    let _ = tokio::join!(udp_server_handle, tcp_server_handle);

    return Ok(());
}
