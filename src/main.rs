pub mod av;
pub mod rpc;
pub mod stream;
pub mod utils;
pub mod version;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    // parse command line arguments
    let cli_args = utils::cli::CommandLines::new(&version::APP_NAME, &version::APP_VERSION);

    // open daily log
    utils::log::open_daily_file_log(&version::APP_NAME, &version::APP_VERSION, &cli_args);

    // prepare stream server
    let (stream_udp_socket, stream_tcp_listener) = stream::server::bind(&cli_args).await?;

    // run stream server
    let stream_handler =
        stream::handler::StreamHandler::new(&cli_args, stream_udp_socket, stream_tcp_listener);
    let stream_handler_arc = std::sync::Arc::new(stream_handler);
    let stream_service = stream::server::run_forever(cli_args.clone(), stream_handler_arc.clone());

    // wait
    let _ = tokio::join!(stream_service);

    Ok(())
}
