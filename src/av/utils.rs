use libc;

use std::ffi::{CStr, CString};

use ffmpeg_sys_next as ffmpeg;

pub fn c_str_to_string(c_str: *const libc::c_char) -> String {
    unsafe { CStr::from_ptr(c_str).to_str().unwrap().to_string() }
}

pub fn str_to_c_str(str: &str) -> CString {
    CString::new(str).expect("could not alloc CString")
}

pub fn ffmpeg_error(func: &str, code: i32) -> String {
    unsafe {
        let msg = c_str_to_string(ffmpeg::strerror(code));
        format!(
            "[ffmpeg_error] code: {}, func: {}, msg: {}",
            code, func, msg
        )
    }
}
