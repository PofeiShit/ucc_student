int a,b;
int main(int argc, char *argv[])
{
    switch(a) {
    case 1:
        b = 10;
        a = 5;
        break;
    case 3:
        b = 30;
        break;
    case 2:
        b = 20;
        break;
    case 5:
        b = 50;
        break;
    }
    b = 60;
    return 0;
}
