#include <cstddef>
#include <iostream>
#include <string>
#include <fstream>
#include "vector.h"

using namespace std;

void DemoVector(){
    Vector<T1> v1(10);
    v1.push_back(1);
    v1.push_back(2);
    v1.push_back(-1);
    v1.push_back(4);
    cout << v1.toString() << endl;
    cout << v1 << endl;
    // cout << "hola" << 5 << endl;
    // cout.operator<<("hola")
    // ==============
    //           cout << 5 << endl;
    //           =========
    //                cout << endl;

    Vector<string> v2(10);
    v2.push_back("Hola");
    v2.push_back("Mundo");
    v2.push_back("!");
    cout << v2 << endl;
    cout << v2.toString() << endl;

    {
        ofstream of("temp.txt");
        of << v1 << endl;
        of << v2 << endl;
    }

    ifstream ifs("temp.txt");
    Vector<T1> v3(10);
    Vector<string> v4(10);
    ifs >> v3 >> v4;

    cout << v3 << endl;
    cout << v4 << endl;
}
