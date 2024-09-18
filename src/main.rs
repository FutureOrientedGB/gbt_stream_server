pub mod av;
pub mod rpc;
pub mod stream;
pub mod utils;
pub mod version;

pub mod gss {
    tonic::include_proto!("gss");
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    // parse command line arguments
    let cli_args = utils::cli::CommandLines::new(&version::APP_NAME, &version::APP_VERSION);

    // open daily log
    utils::log::open_daily_file_log(&version::APP_NAME, &version::APP_VERSION, &cli_args);

    // log panic
    std::panic::set_hook(Box::new(|info: &std::panic::PanicInfo<'_>| {
        tracing::error!("{:?}", info);
    }));

    // serve grpc
    let rpc_addr = format!("{}:{}", &cli_args.host, &cli_args.grpc_port);
    let rpc_service = rpc::server::MyGbtStreamService::new(cli_args.clone());
    match tonic::transport::Server::builder()
        .add_service(gss::gbt_stream_service_server::GbtStreamServiceServer::new(
            rpc_service,
        ))
        .serve(rpc_addr.parse().unwrap())
        .await
    {
        Ok(_) => {}
        Err(e) => {
            tracing::error!("grpc serve error, e: {:?}", e);
        }
    };

    Ok(())
}
