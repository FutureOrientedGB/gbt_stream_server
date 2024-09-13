use local_ip_address::local_ip;

use structopt::StructOpt;

#[derive(Clone, StructOpt)]
pub struct CommandLines {
    #[structopt(long, default_value = "0.0.0.0")]
    pub host: String,

    #[structopt(long, default_value = "")]
    pub my_ip: String,

    #[structopt(long, default_value = "7080")]
    pub grpc_port: u16,

    #[structopt(long, default_value = "10001")]
    pub stream_port_start: u16,

    #[structopt(long, default_value = "20000")]
    pub stream_port_stop: u16,

    #[structopt(long, default_value = "1500")]
    pub socket_recv_buffer_size: usize,
}

impl CommandLines {
    pub fn new(app_name: &str, app_version: &str) -> CommandLines {
        let cli_app = CommandLines::clap().name(app_name).version(app_version);

        let mut results = CommandLines::from_clap(&cli_app.get_matches());
        
        if results.my_ip.is_empty() {
            results.my_ip = local_ip().unwrap().to_string();
        }

        return results;
    }
}
