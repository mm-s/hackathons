#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <cassert>

using namespace std;

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/

//bellman ford: relax all edges N-1 times


void relax(map<int,pair<int,int>>& m, int from, int to) {
    auto f=m.find(from);
    if (f!=m.end()) {
//         cerr << "found " << from << " " << m.size() << endl;
        int d=f->second.second;
        {
        auto t=m.find(to);
        if (t==m.end()) m.emplace(to,pair<int,int>(from,d+1));
        }
        auto t=m.find(to);
        if (t->second.second>=d+1) {
//               cerr << "relaxed " << from << "-" << to << " " << t->second.second << endl;
            t->second.second=d+1;
            t->second.first=from;                
        }
    }

}


vector<int> find_path(const vector<pair<int,int>>& e,int total_nodes, int nfrom, int nto) {
cerr << "find path links " << e.size() << " total_nodes " << total_nodes << " from " << nfrom << " to " << nto << endl;
cerr << "adjacency list" << endl;
for (auto v:e) {
    cerr << "    " << v.first << "-" << v.second << endl;
}
cerr << endl;
    map<int,pair<int,int>> m; //traverse info node - node from, distance
    m[nfrom]=pair<int,int>(-1,0);

    for (int i=0; i<total_nodes-1; ++i) {
        for (auto edge: e) {
            relax(m,edge.first,edge.second);
            relax(m,edge.second,edge.first);
            
            
        }
    }
    
    vector<int> path;
    int cur=nto;
cerr << " nto " << nto << " msize: " << m.size() << " path: " ;
    while (cur!=-1) {
        path.push_back(cur);
        cerr << cur << " "; cerr.flush();
        auto i=m.find(cur);
        if (i==m.end()) {
            cerr << "unexpected " << cur << endl;
            break;
        }
        cur=i->second.first;
    }
cerr << endl;
    return move(path);
}

int main()
{
    int N; // the total number of nodes in the level, including the gateways
    int L; // the number of links
    int E; // the number of exit gateways
    cin >> N >> L >> E; cin.ignore();
    
    vector<pair<int,int>> e;
    
    for (int i = 0; i < L; i++) {
        int N1; // N1 and N2 defines a link between these nodes
        int N2;
        cin >> N1 >> N2; cin.ignore();
        e.push_back(pair<int,int>(N1,N2));
    }

    vector<int> gw;

    for (int i = 0; i < E; i++) {
        int EI; // the index of a gateway node
        cin >> EI; cin.ignore();
        gw.push_back(EI);
    }
    
//    cout << n << " " << n2 << endl;

    // game loop
    while (1) {
        int SI; // The index of the node on which the Skynet agent is positioned this turn
        cin >> SI; cin.ignore();
        //find shortest paths from SI to every gateway
        //select the gateway with the sortest path
        //destroy the first link in the path
        int ms=numeric_limits<int>::max();
        int n=-1;
        int n2=-1;
        pair<int,int> edge;
        for (auto EI:gw) {
            vector<int> p=find_path(e,N,SI,EI);
   // cerr << p.size() << endl;
            if (p.size()<ms) {
                ms=p.size();
                auto t=p.begin();
                auto f=p.begin();
                ++f;
                edge=pair<int,int>(*f,*t);

            }
        }
        for (auto i=e.begin(); i!=e.end(); ++i) {
            if (*i==edge) {
                e.erase(i);
                break;
            }
        }
        
        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;

        cout << edge.first << " " << edge.second << endl; // Example: 0 1 are the indices of the nodes you wish to sever the link between
    }
}
