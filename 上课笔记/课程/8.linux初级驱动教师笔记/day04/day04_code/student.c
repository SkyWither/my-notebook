#include <stdio.h>

//定义人的类型
struct person{
	int age;
	char *name;
};

struct student{
 // struct person p;    //父类，student继承person类型，则在student中可以使用person中的成员
int age;
char *name;
  int sno;
};

struct worker{
	//struct person p;
	int age;
	char *name;
	float gongzi;
};


int main(void)
{
	struct student s;

	s.p.age = 18;
	s.p.name = "小明";
	s.sno  = 1002;

	printf("%d %s  %d\n",s.sno,s.p.name,s.p.age);
	return 0;
}
