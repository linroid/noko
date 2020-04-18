//
// Created by linroid on 2020/4/18.
//
#include <cstdlib>
#include <string>
#include <iostream>
#include <node.h>

int main(int argc, char *argv[]) {

    std::string hello = "Hello from C++";
    std::cout << "Message from native code: " << hello << "\n";
    node::Start(argc, argv);
    return EXIT_SUCCESS;
}
