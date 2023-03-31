from c3ga_py import *


mv1 = Mvec();
mv1[scalar] = 1.0;
mv1[E0] = 42.0;
print("mv1 : ", mv1 )

mv2 = Mvec()
mv2[E0] = 1.0;
mv2[E1] = 2.0;
mv2 += I() + 2*e01();
print("mv2 : " , mv2 );

# some products
print("outer product     : ", (mv1 ^ mv2) )
print("inner product     : ", (mv1 | mv2) )
print("geometric product     : ", (mv1 * mv2) )


# some tools
print("grade : ", mv1.grade())
print("norm : ", mv1.norm())
print("grade of mv2 : ", mv2.grade())


