#if 1
int start(int x)
{
    if(x <= 1)
        return 1;
    else
        return start(x-1) + start(x-2);
    /*if(x > 12)
        return 42;
    else
        return x;*/
}
#endif

#if 0
int start(int x)
{
    return x * 42;
}
#endif

#if 0
typedef struct
{  
    unsigned a : 3;
    int x : 7;
    unsigned b : 13;
} foo;

int start(int x)
{
    foo f;
    f.x = x;
    return baz(&f);
}

int baz(foo *p)
{
    return p->x;
}
#endif