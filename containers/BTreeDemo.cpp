//#include <iostream.h>
#include <time.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include "BTree.h"
#include "traits.h"
#include "../types.h"

const TChar * keys1 = "D1XJ2xTg8zKL9Ahij";
const TChar * keys2 = "0123456789ABCDEFG";
const TChar * keys3 = "DYZakHIMwxVJ2K3ej";


void DemoBTree()
{
    size_t i;
    const T1 BTreeSize = 3;

    // Ascending BTree
    cout<<"===== ASCENDING B-TREE ====="<<endl;
    BTree<BTreeAscendingTrait<TChar>> btAsc(BTreeSize);

    // Insert
    cout<<"\n===== INSERT ====="<<endl;
    for (i = 0; keys1[i]; i++)
    {
        cout << "Insertando " << keys1[i] << endl;
        btAsc.Insert(keys1[i], i*i);
    }

    btAsc.Print(cout);

    // Descending BTree
    cout << "\n===== DESCENDING B-TREE =====" << endl;
    BTree<BTreeDescendingTrait<TChar>> btDesc(BTreeSize);
    
    // Insert
    cout<<"\n===== INSERT ====="<<endl;
    for (i = 0; keys1[i]; i++)
    {
        cout << "Insertando " << keys1[i] << endl;
        btDesc.Insert(keys1[i], i*i);
    }

    btDesc.Print(cout);

    // Search
    cout<<"\n===== SEARCH ====="<<endl;
    for (i = 0; keys2[i]; i++)
    {
        cout << "Buscando " << keys2[i] << " ";
        TLong ObjID = btAsc.Search(keys2[i]);
        if( ObjID != -1 )
            cout << "Encontrado " << keys2[i] << " ID = " << ObjID << endl;
        else
            cout << ", No encontrado " << keys2[i] << endl;
    }

    cout.flush();

    // Remove
    /* for (i = 0; keys3[i]; i++)
    {
        cout << "Removiendo " << keys3[i] << ", ";
        if( btAsc.Remove(keys3[i], -1) )
            cout << keys3[i] << " removido!" << endl;
        else
            cout << "No encontrado " << keys3[i] << endl;
        btAsc.Print(cout);
    } */

    // ForEach
    cout<<"\n=== FOREACH (ASC) ==="<<endl;
    btAsc.ForEach(
        [](decltype(btAsc)::ObjectInfo &info, T1 level)
        {
            for (T1 j = 0; j < level; j++) cout << "  ";
            cout<<info.key<<" (Id="<<info.ObjID<<")"<<endl;
        }
    );

    // FirstThat
    cout<<"\n=== FIRST THAT (ASC) ==="<<endl;
    auto firstfound1 = btAsc.FirstThat(
        [](decltype(btAsc)::ObjectInfo &info, T1) -> bool
        {
            return info.key > 'K';
        });
    if (firstfound1)
        cout<<"Clave encontrada = "<<firstfound1->key<<", Id="<<firstfound1->ObjID<<endl;
    else
        cout<<"Clave no encontrada"<<endl;

    cout.flush();

    // UnifiedLoop (ForEach) 
    cout<<"\n=== UNIFIEDLOOP COMO FOREACH (ASC) ==="<<endl;
    btAsc.UnifiedLoop(
        [](decltype(btAsc)::ObjectInfo& info, T1 level)
        {
            for (T1 j = 0; j < level; j++) cout << "  ";
            cout<<info.key<<" (Id="<<info.ObjID<< ")"<<endl;
        }
    );

    // UnifiedLoop (FirstThat)
    cout<<"\n=== UNIFIEDLOOP COMO FIRSTTHAT (ASC) ==="<<endl;
    auto* found = btAsc.UnifiedLoop(
        [](decltype(btAsc)::ObjectInfo& info, T1) -> TBool
        {
            return info.key > 'J';
        });
    if (found)
        cout<<"Clave encontrada = "<<found->key<<", Id="<<found->ObjID<<endl;
    else
        cout<<"Clave no encontrada"<<endl;

    cout.flush();

    // Iterators
    cout<<"\n=== FORWARD ITERATOR (ASC) ==="<<endl;
    for (auto it = btAsc.begin(); it != btAsc.end(); ++it)
        cout<<"  "<<(*it).key<<" -> "<<(*it).ObjID<<endl;

    cout<<"\n=== BACKWARD ITERATOR (ASC) ==="<<endl;
    for (auto it = btAsc.rbegin(); it != btAsc.rend(); ++it)
        cout<<"  "<<(*it).key<<" -> "<<(*it).ObjID<<endl;

    // Operator >>
    cout<<"\n=== OPERATOR >> ==="<<endl;
    BTree<BTreeAscendingTrait<TChar>> btInput(BTreeSize);
    TString inputData = "A:45 5:56 8:78 g:75";
    cout<<"Input string: \""<<inputData<<"\""<<endl;

    istringstream ss(inputData);
    ss>>btInput;
    cout<<"BTree despues del operador >>:"<<endl;
    for (auto item: btInput)
        cout<<item<<endl;

    cout.flush();

    // Operator <<
    cout<<"\n=== OPERATOR << ==="<<endl;
    cout<<btAsc;
    cout.flush();

    cout<<"\n=== FOREACH NATIVO ==="<<endl;
    for (auto& item : btAsc)
        cout<<item<<endl;

    cout.flush();
}
