// swift-tools-version: 5.9
import PackageDescription

let package = Package(
    name: "RubyVm",
    platforms: [.iOS(.v14)],
    products: [
        .library(
            name: "RubyVm",
            targets: ["RubyVMPlugin"])
    ],
    dependencies: [
        .package(url: "https://github.com/ionic-team/capacitor-swift-pm.git", from: "7.0.0")
    ],
    targets: [
        .target(
            name: "RubyVMPlugin",
            dependencies: [
                .product(name: "Capacitor", package: "capacitor-swift-pm"),
                .product(name: "Cordova", package: "capacitor-swift-pm")
            ],
            path: "ios/Sources/RubyVMPlugin"),
        .testTarget(
            name: "RubyVMPluginTests",
            dependencies: ["RubyVMPlugin"],
            path: "ios/Tests/RubyVMPluginTests")
    ]
)