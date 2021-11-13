int a = 10;
int main(int argc,char * argv[]) 
{
    {   
        int a = 20;
        int b[10] = {1,2};
        { 
            int a = 30;
            a = 40; 
        }
        a = 50; 
    }
    a = 60; 
    return 0;
}
