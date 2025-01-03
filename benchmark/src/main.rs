use clap::Parser;

use libloading::Symbol;

use sgx_urts_sys::{sgx_create_enclave, sgx_enclave_id_t, sgx_launch_token_t};

use std::{
    error::Error,
    ffi::{c_char, c_int, CString},
    fmt::{Display, Formatter},
    path::{Path, PathBuf},
    ptr::null_mut,
};

#[derive(Debug)]
struct Lib<'l> {
    run: Symbol<
        'l,
        extern "C" fn(sgx_enclave_id_t, u64, c_int, u64, *mut u64, *mut u64, *mut u64) -> c_int,
    >,
    setup: Symbol<'l, extern "C" fn(sgx_enclave_id_t, u64, *const *const c_char) -> c_int>,
    destroy: Symbol<'l, extern "C" fn(sgx_enclave_id_t) -> c_int>,
}

impl<'l> Lib<'l> {
    fn new(lib: &'l libloading::Library) -> Result<Self, Box<dyn Error>> {
        unsafe {
            Ok(Self {
                run: lib.get(b"tlblur_benchmark_run")?,
                setup: lib.get(b"tlblur_benchmark_setup")?,
                destroy: lib.get(b"tlblur_benchmark_destroy")?,
            })
        }
    }
}

#[derive(Debug, Clone)]
struct Results {
    timings: Vec<u64>,
    data_accesses: Vec<u64>,
    code_accesses: Vec<u64>,
}

fn average(numbers: &[u64]) -> u64 {
    let sum: u64 = numbers.iter().sum();
    let count = numbers.len() as u64;
    sum / count
}

impl Results {
    fn average_cycles(&self) -> u64 {
        // HACK: Skip the first one because it's an outlier
        average(&self.timings[1..])
    }

    fn average_data(&self) -> u64 {
        average(&self.data_accesses)
    }

    fn average_code(&self) -> u64 {
        average(&self.code_accesses)
    }

    fn write_csv(&self, path: impl AsRef<Path>) -> Result<(), Box<dyn Error>> {
        let mut writer = csv::Writer::from_path(path)?;
        writer.write_record(&["cycles", "data", "code"])?;
        for i in 0..self.timings.len() {
            writer.write_record(&[
                &self.timings[i].to_string(),
                &self.data_accesses[i].to_string(),
                &self.code_accesses[i].to_string(),
            ])?;
        }
        Ok(())
    }
}

#[derive(Debug, Clone, Copy)]
enum BenchmarkError {
    // TODO: add different errors
    Unknown,
}

impl Display for BenchmarkError {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        match self {
            BenchmarkError::Unknown => f.write_str("unknown error"),
        }
    }
}

impl Error for BenchmarkError {}

impl From<i32> for BenchmarkError {
    fn from(_value: i32) -> Self {
        BenchmarkError::Unknown
    }
}

#[derive(Debug)]
struct Benchmark<'a, 'l> {
    eid: sgx_enclave_id_t,
    lib: &'a mut Lib<'l>,
    untrusted_loop: bool,
    warmup: usize,
}

impl<'a, 'l> Benchmark<'a, 'l> {
    fn new(
        eid: sgx_enclave_id_t,
        lib: &'a mut Lib<'l>,
        args: &[&str],
        untrusted_loop: bool,
        warmup: usize,
    ) -> Result<Self, BenchmarkError> {
        let args: Vec<_> = args.iter().map(|&a| CString::new(a).unwrap()).collect();
        let args: Vec<_> = args.iter().map(|a| a.as_ptr()).collect();

        let res = (lib.setup)(eid, args.len() as u64, args.as_ptr());
        if res != 0 {
            println!("Error: {res}");
            return Err(res.into());
        }
        Ok(Self {
            eid,
            lib,
            untrusted_loop,
            warmup,
        })
    }

    fn run(&mut self, iterations: usize) -> Result<Results, BenchmarkError> {
        let mut timings = vec![0; iterations];
        let mut data_accesses = vec![0; iterations];
        let mut code_accesses = vec![0; iterations];
        let res = (self.lib.run)(
            self.eid,
            iterations as u64,
            if self.untrusted_loop { 1 } else { 0 },
            self.warmup as u64,
            timings.as_mut_ptr(),
            data_accesses.as_mut_ptr(),
            code_accesses.as_mut_ptr(),
        );
        if res != 0 {
            println!("Error: {res}");
            return Err(res.into());
        }
        Ok(Results {
            timings,
            data_accesses,
            code_accesses,
        })
    }
}

impl<'a, 'l> Drop for Benchmark<'a, 'l> {
    fn drop(&mut self) {
        assert!((self.lib.destroy)(self.eid) == 0)
    }
}

#[derive(Parser, Debug)]
#[command(author, version, about, long_about = None)]
struct Args {
    #[arg(long)]
    so: String,

    #[arg(short, long)]
    enclave: String,

    #[arg(short, long)]
    iterations: usize,

    #[arg(long, value_parser, num_args = 1.., value_delimiter = ' ')]
    args: Vec<String>,

    #[arg(short, long)]
    output: PathBuf,

    #[arg(short, long, default_value_t = false)]
    untrusted_loop: bool,

    #[arg(short, long, default_value_t = 0)]
    warmup: usize,
}

fn main() -> Result<(), Box<dyn Error>> {
    let args = Args::parse();

    let library = unsafe { libloading::Library::new(&args.so)? };
    let mut lib = Lib::new(&library)?;

    let mut token: sgx_launch_token_t = [0; 1024];
    let mut updated = 0;
    let mut eid: sgx_enclave_id_t = 0;

    unsafe {
        let enclave_so = CString::new(args.enclave)?;
        println!(
            "Creating enclave... result: {:x}",
            sgx_create_enclave(
                enclave_so.as_ptr(),
                1,
                &mut token,
                &mut updated,
                &mut eid,
                null_mut(),
            )
        );

        println!("Created enclave with eid {eid}");
    }

    let benchmark_args: Vec<_> = args.args.iter().map(|a| a.as_str()).collect();
    let mut benchmark = Benchmark::new(
        eid,
        &mut lib,
        &benchmark_args,
        args.untrusted_loop,
        args.warmup,
    )?;
    let results = benchmark.run(args.iterations)?;

    println!("average cycles: {}", results.average_cycles());
    println!("average code: {}", results.average_code());
    println!("average data: {}", results.average_data());

    results.write_csv(&args.output)?;

    Ok(())
}
