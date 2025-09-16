use std::path::Path;
use std::env;
use std::fs;

const TOML_VSF_NODE: &str = "vsf"; 
const TOML_VSF_PATH_NODE: &str = "path";
const TOML_TARGET_NODE: &str = "target";
const TOML_TARGET_VENDOR_NODE: &str = "vendor";
const TOML_TARGET_MODEL_NODE: &str = "model";
const TOML_TARGET_FLAGS_NODE: &str = "flags";

const PERIPHERIALS: [&'static str; 2] = ["gpio", "usart"];

fn main() {
    let manifest_dir = env::var("CARGO_MANIFEST_DIR").unwrap();
    let manifest_path = Path::new(&manifest_dir).join("Cargo.toml");

    let cargo_toml_content = fs::read_to_string(manifest_path).unwrap();
    let cargo_toml: toml::Value = toml::from_str(&cargo_toml_content).unwrap();

    let mut path = String::from("");
    let mut vendor = String::from("");
    let mut model = String::from("");
    let mut flags:Vec<String> = vec![];

    if let Some(vsf_node) = cargo_toml.get(TOML_VSF_NODE) {
        if let Some(path_node) = vsf_node.get(TOML_VSF_PATH_NODE) {
            if let Some(path_str) = path_node.as_str() {
                path = String::from(path_str);
            }
        }

        if let Some(target_node) = vsf_node.get(TOML_TARGET_NODE) {
            if let Some(vendor_node) = target_node.get(TOML_TARGET_VENDOR_NODE) {
                if let Some(vendor_str) = vendor_node.as_str() {
                    vendor = String::from(vendor_str);
                }
            }

            if let Some(model_node) = target_node.get(TOML_TARGET_MODEL_NODE) {
                if let Some(model_str) = model_node.as_str() {
                    model = String::from(model_str);
                }
            }

            if let Some(flags_node) = target_node.get(TOML_TARGET_FLAGS_NODE) {
                if let Some(flags_arr) = flags_node.as_array() {
                    for flag in flags_arr {
                        let flag_option_str = flag.as_str();
                        if let Some(flag_str) = flag_option_str {
                            flags.push(String::from(flag_str));
                        }
                    }
                }
            }
        }
    }

    if path.starts_with(".") {
        path.insert(0, '/');
        path.insert_str(0, &manifest_dir);
    }
    if !path.ends_with("/") {
        path.push('/');
    }

    println!("cargo:warning=path: {path}");
    println!("cargo:warning=target: {vendor}.{model}");
    println!("cargo:warning=flags:");
    for flag in &flags {
        println!("cargo:warning=\t{flag}");
    }


    let mut builder = bindgen::Builder::default()
                    .header("".to_string() + &path + "source/hal/driver/driver.h")
                    .blocklist_item("__.*")
                    .default_enum_style(bindgen::EnumVariation::Rust {
                        non_exhaustive: false,
                    })
                    .clang_arg("-D".to_string() + "__" + &vendor + "__")
                    .clang_arg("-D".to_string() + "__" + &model + "__")
                    .clang_arg("-I".to_string() + &path + "source/shell/hal/rust-embedded-hal/inc")
                    .clang_arg("-I".to_string() + &path + "source");

    for flag in &flags {
        builder = builder.clang_arg(flag);
    }

    let bindings = builder.generate().expect("Failed to generate rust bindings");

    let pathbuf = Path::new(&manifest_dir).join("vsf_hal.rs");
    bindings.write_to_file(&pathbuf).unwrap();

    let bindings_content = fs::read_to_string(&pathbuf).unwrap();
    let bindings_lines = bindings_content.lines().collect();

    for peripherial in PERIPHERIALS {
        enable_peripherial(&bindings_lines, peripherial);
    }
}

fn enable_peripherial(lines: &Vec<&str>, name: &str) {
    let mut prefix_str = "VSF_HW_".to_string();
    prefix_str.push_str(&String::from(name).to_uppercase());
    let mut mask_str = String::from(&prefix_str);
    mask_str.push_str("_MASK");
    let mut count_str = String::from(&prefix_str);
    count_str.push_str("_COUNT");

    let mask_option = extract_const_integer(lines, &mask_str);
    if let Some(mask) = mask_option {
        println!("cargo:warning={name}_mask: 0x{mask:X}");

        for index in 0..32 {
            if mask & (1 << index) != 0 {
                println!("cargo:warning={name}: enable {name}{index}");
                println!("cargo:rustc-cfg=vsf_{name}{index}_enabled");
            }
        }
    } else {
        let count_option = extract_const_integer(lines, &count_str);
        if let Some(mut count) = count_option {
            println!("cargo:warning={name}_count: {count}");

            let mut index = 0;
            while count != 0 {
                println!("cargo:warning={name}: enable {name}{index}");
                println!("cargo:rustc-cfg=vsf_{name}{index}_enabled");
                index += 1;
                count -= 1;
            }
        }
    }
}

fn extract_const_integer(lines: &Vec<&str>, name: &str) -> Option<i32> {
    let mut result: Option<i32> = None;

    for line in lines {
        if let Some((cur_name, cur_value)) = extract_constant_value(line) {
            if name == cur_name {
                let value = String::from(cur_value).parse::<i32>().unwrap();
                result = Some(value);
                break;
            }
        }
    }
    result
}

fn extract_constant_value(line: &str) -> Option<(&str, &str)> {
    let parts: Vec<&str> = line.split_whitespace().collect();
    // pub const CONST_NAME: TYPE = CONST_VALUE;
    if parts.len() >= 6 && parts[0] == "pub" && parts[1] == "const" {
        let name = parts[2].trim_end_matches(':');
        let value = parts[5].trim_end_matches(';');
        Some((name, value))
    } else {
        None
    }
}
