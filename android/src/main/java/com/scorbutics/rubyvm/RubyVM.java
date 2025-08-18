package com.scorbutics.rubyvm;

import com.getcapacitor.Logger;

public class RubyVM {

    public String echo(String value) {
        Logger.info("Echo", value);
        return value;
    }
}
