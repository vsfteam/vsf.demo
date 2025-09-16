use std::path::Path;
use std::env;
use std::fs;

const TOML_VSF_NODE: &str = "vsf"; 
const TOML_VSF_PATH_NODE: &str = "path";
const TOML_TARGET_NODE: &str = "target";
const TOML_TARGET_VENDOR_NODE: &str = "vendor";
const TOML_TARGET_MODEL_NODE: &str = "model";
const TOML_TARGET_FLAGS_NODE: &str = "flags";

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
    bindings.write_to_file(Path::new(&manifest_dir).join("vsf_hal.rs")).expect("Fail to write result to rust file");
}
