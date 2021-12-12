struct test {
	char a;
	int b;
} t;
struct Packet
{
	int len;
	char data[];
} p;
struct Data {
	struct {
        int a;
 		int b;
 	};
 	int c;
};
struct Data1
{
    int a;
    struct {
        int b1;
        int b2;
        int b3;
    };
    int c;
    struct {
        int d1;
        int d2;
        int d3;
    } d;
};
struct Data2
{
	char ch;
	int num;
} dt2;
struct Data dt;
struct Data1 dt1;
int a;
void main()
{
	a = dt.b;
	a = dt1.b3;
	a = dt1.d.d3;
}