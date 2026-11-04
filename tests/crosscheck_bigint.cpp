// crosscheck_bigint.cpp - doc lenh tu stdin (add/sub/mul/div/mod/shl/shr/dec/bl/msm), in ket qua.
// Dung cung crosscheck_bigint.py de doi chieu 30000 phep tinh voi so nguyen Python.
// Bien dich: g++ -std=c++14 -O2 -o crosscheck_bigint crosscheck_bigint.cpp
#include <iostream>
#include <chrono>
#include "../project_01_source/bigint.cpp"
// doc: op hexA hexB  -> in ket qua hex / dec
int main(){
  std::string op,a,b;
  while(std::cin>>op>>a>>b){
    big_t x=big_t::from_hex(a), y=big_t::from_hex(b);
    if(op=="add") std::cout<<(x+y).to_hex();
    else if(op=="sub") std::cout<<(x-y).to_hex();
    else if(op=="mul") std::cout<<(x*y).to_hex();
    else if(op=="div") std::cout<<(x/y).to_hex();
    else if(op=="mod") std::cout<<(x%y).to_hex();
    else if(op=="shl") std::cout<<(x<<(size_t)y.digits[0]).to_hex();
    else if(op=="shr") std::cout<<(x>>(size_t)y.digits[0]).to_hex();
    else if(op=="dec") std::cout<<x.to_dec();
    else if(op=="bl") std::cout<<x.bit_length();
    else if(op=="msm") std::cout<<mod_small(x,(uint32_t)y.digits[0]);
    std::cout<<"\n";
  }
  // benchmark divmod 1024-bit product by 512-bit modulus
  big_t p=big_t::from_hex("c"+std::string(127,'f')); big_t q=p-big_t::from_u64(12345);
  big_t t=q*q; auto t0=std::chrono::steady_clock::now(); int n=2000; big_t r;
  for(int i=0;i<n;i++){ r=t%p; t=r*q; }
  double s=std::chrono::duration<double>(std::chrono::steady_clock::now()-t0).count();
  std::cerr<<"512-bit mulmod: "<<s/n*1e6<<" us each; est modexp(512-bit exp)= "<<s/n*768*1000<<" ms\n";
}

