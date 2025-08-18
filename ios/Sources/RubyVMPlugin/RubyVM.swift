import Foundation

@objc public class RubyVM: NSObject {
    @objc public func echo(_ value: String) -> String {
        print(value)
        return value
    }
}
