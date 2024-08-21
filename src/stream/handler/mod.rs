use crate::utils::cli::CommandLines;

pub struct StreamHandler {
    pub ip: String,
    pub port: u16,
    pub stream_udp_socket: tokio::net::UdpSocket,
    pub stream_tcp_listener: tokio::net::TcpListener,
}

impl StreamHandler {
    pub fn new(
        cli_args: &CommandLines,
        stream_udp_socket: tokio::net::UdpSocket,
        stream_tcp_listener: tokio::net::TcpListener,
    ) -> Self {
        StreamHandler {
            ip: cli_args.my_ip.clone(),
            port: cli_args.stream_port,
            stream_udp_socket: stream_udp_socket,
            stream_tcp_listener: stream_tcp_listener,
        }
    }
}
