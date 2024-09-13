use tonic::{Request, Response, Status};

use crate::gss::{ResponseCode, BindStreamPortRequest, BindStreamPortResponse};
use crate::rpc::server::MyGbtStreamService;
use crate::stream;

impl MyGbtStreamService {
    pub async fn rpc_bind_stream_port(
        &self,
        request: Request<BindStreamPortRequest>,
    ) -> Result<Response<BindStreamPortResponse>, Status> {
        let mut reply = BindStreamPortResponse::default();

        let port = self.pop_port();
        if port == 0 {
            reply.code = ResponseCode::NoPortsFree.into();
            reply.message = ResponseCode::NoPortsFree.as_str_name().to_string();
            return Ok(Response::new(reply));
        }

        let is_running = std::sync::Arc::new(true);

        // bind
        match stream::server::bind(&self.cli_args.host, port).await {
            Err(e) => {
                tracing::error!("stream::server::bind error, e: {:?}", &e);
                reply.code = ResponseCode::BindPortError.into();
                reply.message = e.to_string();
                return Ok(Response::new(reply));
            }
            Ok((stream_udp_socket, stream_tcp_listener)) => {
                // listen
                let stream_handler = stream::handler::StreamHandler::new(
                    self.cli_args.my_ip.clone(),
                    port,
                    stream_udp_socket,
                    stream_tcp_listener,
                );

                let arc_stream_handler = std::sync::Arc::new(stream_handler);
                match stream::server::run_forever(
                    is_running.clone(),
                    self.cli_args.socket_recv_buffer_size,
                    arc_stream_handler,
                )
                .await
                {
                    Err(e) => {
                        tracing::error!("stream::server::run_forever error, e: {:?}", &e);
                        reply.code = ResponseCode::RunStreamServiceError.into();
                        reply.message = e.to_string();
                        return Ok(Response::new(reply));
                    }
                    Ok((udp_join_handle, tcp_join_handle)) => {
                        self.push_task(port, is_running.clone(), udp_join_handle, tcp_join_handle);

                        reply.code = ResponseCode::Ok.into();
                        reply.message = String::new();
                        reply.media_server_ip = self.cli_args.my_ip.clone();
                        reply.media_server_port = port as u32;
                        return Ok(Response::new(reply));
                    }
                }
            }
        }
    }
}
