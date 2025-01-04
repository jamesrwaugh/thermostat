fn main() {
    println!("cargo:rustc-link-search=/home/james/Desktop/Git/squaredel/thermostat/software/builddir/src/avr-driver");
    println!("cargo:rustc-link-lib=thermo-avr-driver");
}
