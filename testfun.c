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
