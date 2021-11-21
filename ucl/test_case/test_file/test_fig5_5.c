int a , b, c;
int main(int argc,char * argv[])
{
    if (a + b) {
        c = 1;
    } else {
        c = 2;
    }
    if (a && b) {
        c = 3;
    } else {
        c = 4;
    }
    if (a & b) {
        c = 5;
    } else {
        c = 6;
    }
    c = a || b;
    return 0;
}
