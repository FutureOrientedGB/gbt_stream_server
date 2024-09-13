use chrono;

use time;

use tracing_appender;
use tracing_subscriber;

use crate::utils::ansi_color as Color;

use super::cli::CommandLines;

pub fn open_daily_file_log(app_name: &str, app_version: &str, cli_args: &CommandLines) {
    let mut log_dir = std::env::current_exe()
        .unwrap()
        .parent()
        .unwrap()
        .to_owned();
    log_dir.push("log");

    tracing_subscriber::fmt()
        // .json()
        .with_writer(tracing_appender::rolling::daily(
            &log_dir,
            format!("{app_name}.{port}.log", port = cli_args.grpc_port),
        ))
        .with_max_level(tracing::Level::INFO)
        .with_timer(tracing_subscriber::fmt::time::OffsetTime::new(
            time::UtcOffset::from_hms(8, 0, 0).unwrap(),
            time::macros::format_description!(
                "[year]-[month]-[day] [hour]:[minute]:[second].[subsecond]"
            ),
        ))
        .with_line_number(true)
        .with_thread_ids(true)
        .with_ansi(true)
        .init();

    log_dir.push(format!(
        "{app_name}.{port}.log.{date}",
        port = cli_args.grpc_port,
        date = chrono::Local::now().format("%Y-%m-%d")
    ));
    println!(
        "{}logging to: {}{}",
        Color::PURPLE,
        log_dir.to_str().unwrap(),
        Color::RESET
    );

    tracing::info!(
        "start services{}
╔════════════════════════════════════════════════════════════╗
║      ┌─┐┌┐┌┬┐  ┌─┐┌┬┐┬─┐┌─┐┌─┐┌┬┐  ┌─┐┌─┐┬─┐┬  ┬┌─┐┬─┐     ║
║      │ ┬├┴┐│   └─┐ │ ├┬┘├┤ ├─┤│││  └─┐├┤ ├┬┘└┐┌┘├┤ ├┬┘     ║
║      └─┘└─┘┴   └─┘ ┴ ┴└─└─┘┴ ┴┴ ┴  └─┘└─┘┴└─ └┘ └─┘┴└─     ║
║════════════════════════════════════════════════════════════║
║                                                            ║
║ git: https://github.com:FutureOrientedGB/gbt_stream_server ║
║                                                            ║
║ version: {:<49} ║
║                                                            ║
║ host: {:<52} ║
║ my_ip: {:<51} ║
║ stream_port_start: {:<39} ║
║ stream_port_stop: {:<40} ║
║ grpc_port: {:<47} ║
║ socket_recv_buffer_size: {:<33} ║
╚════════════════════════════════════════════════════════════╝{}",
        Color::CYAN,
        app_version,
        &cli_args.host,
        &cli_args.my_ip,
        &cli_args.stream_port_start,
        &cli_args.stream_port_stop,
        &cli_args.grpc_port,
        &cli_args.socket_recv_buffer_size,
        Color::RESET
    );
}
