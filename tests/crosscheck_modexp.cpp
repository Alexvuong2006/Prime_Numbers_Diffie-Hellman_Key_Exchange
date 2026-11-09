// crosscheck_modexp.cpp - doc "base exp mod" (hex) tu stdin, in (base^exp mod mod) dang hex.
// Dung cung crosscheck_modexp.py de doi chieu voi pow(b, e, m) cua Python.
// Bien dich: g++ -std=c++14 -O2 -o crosscheck_modexp crosscheck_modexp.cpp
#include <iostream>
#include <chrono>
#include "../project_01_source/dh.cpp"

int main()
{
    std::string a, e, m;
    while (std::cin >> a >> e >> m) {
        big_t r = modular_exponentiation(big_t::from_hex(a), big_t::from_hex(e), big_t::from_hex(m));
        std::cout << r.to_hex() << "\n";
    }
    return 0;
}
