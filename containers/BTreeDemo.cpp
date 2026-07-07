#include <cctype>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include "BTree.h"
#include "traits.h"

using Trait = BTreeTrait<char, Ref>;
using BT = BTree<Trait>;
using namespace std;

//const char * keys="CDAMPIWNBKEHOLJYQZFXVRTSGU";
const char * keys1 = "D1XJ2xTg8zKL9AhijOPQcEowRSp0NbW567BUfCqrs4FdtYZakHIuvGV3eMylmn";
const char * keys2 = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
const char * keys3 = "DYZakHIUwxVJ203ejOP9Qc8AdtuEop1XvTRghSNbW567BfiCqrs4FGMyzKLlmn";

const int BTreeSize = 3;
static void concurrencyWorker(BT& tree, Ref workerId)
{
       for( int i = 0; i < 60; i++ )
               tree.Insert(char('a' + ((workerId * 7 + i) % 26)), workerId);
}

int main()
{
       BT bt (BTreeSize);
       string keys = keys1;
       for (size_t i = 0; i < keys.size(); i++)
       {
               //cout<<"Inserting "<<keys1[i]<<endl;
               bt.Insert(keys[i], Ref(i*i));
               //bt.Print(cout);
       }
       cout << "BTree\n";
       bt.Print(cout);
       cout << "size=" << bt.size() << " height=" << bt.height()
            << " order=" << bt.GetOrder() << "\n";

       cout << "search Z=" << bt.Search('Z') << "\n";

       int letters = 0;
       bt.ForEach([](BT::ObjectInfo &info, int, int &count) {
               if( isalpha((unsigned char)info.key) )
                       count++;
       }, letters);
       cout << "letters=" << letters << "\n";

       BT::ObjectInfo *found = bt.FirstThat([](BT::ObjectInfo &info, int, char target) {
               return info.key == target;
       }, 'Q');
       cout << "firstThat Q=" << (found ? found->ObjID : -1) << "\n";

       string forward;
       for( auto &info : bt )
               forward += info.key;
       cout << "forward=" << forward << "\n";

       string backward;
       for( auto it = bt.rbegin(); it != bt.rend(); ++it )
               backward += it->key;
       cout << "backward=" << backward << "\n";

       stringstream ss;
       ss << bt;
       BT copy(BTreeSize);
       ss >> copy;
       cout << "operator io search Z=" << copy.Search('Z') << "\n";

       BT concurrent(BTreeSize);
       vector<thread> threads;
       for( Ref i = 0; i < 4; i++ )
               threads.emplace_back(concurrencyWorker, ref(concurrent), i + 1);
       for( auto &thread : threads )
               thread.join();
       cout << "concurrent size=" << concurrent.size() << "\n";
       /*for (i = 0; keys2[i]; i++)
       {
               cout << "Searching " << keys2[i] << " ";
               long ObjID = bt.Search(keys2[i]);
               if( ObjID != -1 )
                       cout << "Achei " << keys2[i] << " ID = " << ObjID << endl;
               else
                       cout <<"Nao achei!" << keys2[i] << endl;
       }*/
       /*cout.flush();

       for (i = 0; keys3[i]; i++)
       {
               cout << "Removing " << keys3[i] << " ";
               if( bt.Remove(keys3[i], -1) )
                       cout << keys3[i] << " removido !" << endl;
               else
                       cout <<"Nao achei!" << keys3[i] << endl;
               bt.Print(cout);
       }
       bt.Print(cout);
       cout.flush();*/
       return 0;
}









/*const char * keys="CDAMPIWNBKEHOLJYQZFXVRTSGU";
const char * keys2="CDAMPIWNBKEHOLJYQZFXVRTSGU";
const int BTreeSize = 3;
main (int argc, char * argv)
{
       //__int64 li;
       BTree <__int64> bt (BTreeSize);
       for (register int i = 0; i < 1000000; i++)
       {
               //cout<<"Inserting "<<keys[i]<<endl;
               bt.Insert(i, i-1);
               //bt.Print(cout);
       }

       for (i = 0; i < 1000; i++)
       {
               __int64 key = 975000+(::rand()%50000);
               //cout << "Searching " << (long)key << " ";
               long ObjID = bt.Search(key);
               if( ObjID != -1 )
                       cout << "Achei " << (long)key << " ID = " << ObjID << endl;
               else
                       cout <<"  Nao achei!" << (long)key << endl;
       }
       cout.flush();

       return 1;
}*/



/*const int BTreeSize = 3;
main (int argc, char * argv)
{
       int result, i;
       BTree <LONGLONG> bt(BTreeSize);
       result = bt.Create ("ernesto3-string-btree-start.dat",ios::in|ios::out);
       if (!result) { cout<<"Please delete testbt.dat"<<endl;return 0; }
       srand( (unsigned)time( NULL ) );
       LARGE_INTEGER key;
       for (i = 0; i < 1000000; i++)
       {
               //cout<<"Inserting "<<keys[i]<<endl;
               char strTmp[50];
               key.LowPart = rand();
               key.HighPart = rand();
               std::string str(strTmp);
               result = bt.Insert(key.QuadPart, i);
               //bt.Print(cout);
               if( i % 100000 == 0 )
               {       cout << i << endl; cout.flush();        }
       }
       //cout << "Searching D " << bt.Search();
       //bt.Search(1,1);
       cout.flush();
       return 1;
}*/
