export function is_prime(x : i32) : i32
{
    var divisor = 1;

    for(var i = 2; i < x; i++)
    {
        if((x % i) == 0)
        {
            divisor = i;
            break;
        }
    }

    return divisor;
}

