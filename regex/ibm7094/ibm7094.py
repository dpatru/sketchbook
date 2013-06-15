"""
http://research.microsoft.com/en-us/um/people/gbell/Computer_Structures__Readings_and_Examples/00000548.htm

Instructions used in thompson.s:

What do we mean by:
insn   insn<S, 1:35>    I guess S means separate sign bit
Y      Y<21:35> := insn<21:35>
T      T<18:20> := insn<18:20>
op     op<S, 1:11> := insn<S, 1:11>
hi_op  hi_op<0:2> := insn<S, 1, 2>

AC     AC<Q,P,S,1:35>
ACl    ACl<P, 1:35> := AC<P, 1:35>
ACs    ACs<S, 1:35> := AC<S, 1:35>
D      D<3:17> := insn<3:17>   decrement part of insn
IC     IC<3:17>   instruction counter 
M[e]   M[0:32767]<S, 1:35>
M[Y]   memory?
XR[T]  XR[1:7]<3:17>

indirect := insn<12:13> = 11
e      e<21:35> := (not indirect -> e';
                    indirect -> insn<18:35> <- M[e']<18:35>; next e')
e'     ...

next foo
foo<lo:hi>
foo+bar, foo-bar, foo >= bar

acl Add and Carry Logical word
                           (:= op = 361) -> (ACl <- ACl + M[e])
axc Address to indeX Complement
                           (:= op = -774) -> (XR[T] <- 2**15 - Y)
cal Clear and Add Logical  op:-500: AC <- 0; next ACl <- ACl+M[e]
cla Clear and Add          op:500:  AC <- 0; next ACs <- AC+M[e]
lac Load Complement of Address in index
                           (:= op = 535) -> (XR[T] <- 2**15 - M[Y]<21:35>)
pac Place Complement of Address in index
                           (:= op = 737) -> (XR[T] <- 2**15 - AC<21:35>)
pca Place Complement of index in Address
                           (:= op = 756) -> (AC <- 0; next AC<21:35> <- 2**15 - XR[T])
sca Store Complement of index in Address
                           (:= op = 636) -> (M[Y]<21:35> <- 2**15 - XR[T])
slw Store Logical Word     (:= op = 602) -> (M[e] <- ACl)
tra TRAnsfer               (:= op = 20) -> (IC <- e)
tsx Transfer and Set indeX (:= op = 74) -> (XR[T] <- 2**15 - IC; IC <- Y)
txi Transfer with indeX Incremented
                           (:= hi_op = 1) -> (XR[T] <- XR[T] + D; IC <- Y)
txh Transfer on indeX High (:= hi_op = 3) -> ((D < XR[T]) -> IC <- Y)
txl Transfer on indeX Low or equal
                           (:= hi_op = -3) -> ((D >= XR[T]) -> IC <- Y)
"""

class CPU:

    def __init__(self):
        self.M = [0] * 32768    # memory
        self.AC = 0             # accumulator
        self.XR = [0] * 8       # index registers
        self.pc = 0
        
    def step(self):
        insn = self.M[self.pc]
        self.pc += 1
        prefix = (insn >> 0) & ~(~0 << 3)
        decr   = (insn >> 3) & ~(~0 << 15)
        tag    = (insn >> 18) & ~(~0 << 3)
        addr   = (insn >> 21) & ~(~0 << 15)
        effective_addr = addr - self.XR[tag]
        
