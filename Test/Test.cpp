#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <string>
using namespace std;

class Person
{
public:
    string name;
    int age;
    double score;
};

int main()
{
    Person Person;
    Person.name = "Tom";
    cout << Person.name << endl;
    return 0;
}