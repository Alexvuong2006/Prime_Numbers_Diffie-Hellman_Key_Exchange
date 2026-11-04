# Doi chieu BigInt (C++) voi Python int. Chay: python crosscheck_bigint.py ./crosscheck_bigint
import random,subprocess,sys
random.seed(7)
ops=[];exp=[]
def h(v): return format(v,'x')
for _ in range(3000):
    bits=random.choice([1,31,64,65,127,128,200,512,700,1000])
    a=random.getrandbits(bits) ; b=random.getrandbits(random.choice([1,32,64,100,300,512]))|1
    if random.random()<0.1: a=2**random.choice([64,128,512,1000])-random.choice([0,1])
    ops.append(f"add {h(a)} {h(b)}");exp.append(h(a+b))
    big,small=(a,b) if a>=b else (b,a)
    ops.append(f"sub {h(big)} {h(small)}");exp.append(h(big-small))
    a5=a%(1<<1000); b5=b%(1<<1000)
    ops.append(f"mul {h(a5)} {h(b5)}");exp.append(h(a5*b5))
    ops.append(f"div {h(a)} {h(b)}");exp.append(h(a//b))
    ops.append(f"mod {h(a)} {h(b)}");exp.append(h(a%b))
    sh=random.randint(0,300)
    ops.append(f"shl {h(a)} {h(sh)}");exp.append(h((a<<sh)%(1<<2048)))
    ops.append(f"shr {h(a)} {h(sh)}");exp.append(h(a>>sh))
    ops.append(f"dec {h(a)} 0");exp.append(str(a))
    ops.append(f"bl {h(a)} 0");exp.append(str(a.bit_length()))
    m=random.choice([2,3,7,1000003,4294967291]); ops.append(f"msm {h(a)} {h(m)}");exp.append(str(a%m))
r=subprocess.run([sys.argv[1]],input="\n".join(ops)+"\n",capture_output=True,text=True)
out=r.stdout.split("\n")
bad=0
for i,(o,e) in enumerate(zip(ops,exp)):
    if out[i]!=e:
        bad+=1
        if bad<=5: print("MISMATCH",o[:80],"got",out[i][:60],"exp",e[:60])
print("total",len(ops),"mismatch",bad); print(r.stderr)

