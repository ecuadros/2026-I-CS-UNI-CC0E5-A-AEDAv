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

    ofstream of("temp.txt");
    of << v1 << endl;
    of << v2 << endl;
    // of.close();

    // Vector vacio
    Vector<T1> v3(10);
    cout << v3 << endl;

    // Vector in
    Vector<int> v_cin(10);
    cout << "Numeros separados por coma : ";
    cin >> v_cin;
    cout << "v_cin: " << v_cin << endl;

    ifstream ifs("temp.txt");
    Vector<int> v_ifs(10);
    ifs >> v_ifs;
    cout << "temp.txt: " << v_ifs << endl;
    
}