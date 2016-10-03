/*
Google HashCode Feb-2016
Warm up problem

Compile:
	g++ -std=c++14 main.cpp 

Run:
	time cat learn_and_teach.in | ./a.out 

Output files:
	paint.out //render after executing commands
	commands.out  //list of commands

Author: Marcos Mayorga mm@mm-studios.com

run time: 1m40.550s @ 1x4800 bogomips
*/
#include <vector>
#include <cmath>
#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <stack>
#include <limits>

#include <vector>
#include <iostream>
#include <string>
#include <map>

using namespace std;

struct edge;

struct vertex {
	vertex(int id_): id(id_) {
	}
	vector<edge*> e;
	int id;
};

struct edge {  //from, to
	edge(const vertex* f, const vertex* t): from(f), to(t) {
	}
	const vertex* from;
	const vertex* to;

	string key;
};


struct graph {

	map<int,vertex*> V;
	map<string,edge*> E;

	graph(vector<string> al) {  //v1 v2
		for (auto& l:al) {
			int v1=-1;
			int v2=-1;
			istringstream is(l);
			is >> v1;
			is >> v2;
			add(v1,v2);
		}
	}

	void dot(ostream& os) const {
		os << "digraph {" << endl;
		for (auto e: E) {
			os << e.second->from->id << " -> " << e.second->to->id << endl;
		}
		os << "}" << endl;
	}	

	vertex* add(int iv) {
		auto i=V.find(iv);
		if (i==V.end()) {
			V.emplace(iv,new vertex(iv));
			i=V.find(iv);
		}
		return i->second;
	}

	void add(int iv1, int iv2) {
		if (iv1<0 || iv2<0) return;
		ostringstream os;
		os << iv1 << " " << iv2;
		auto v1=add(iv1);
		auto v2=add(iv2);
		edge* e=new edge(v1,v2);
		E.emplace(os.str(),e);
		v1->e.push_back(e);

	}

	~graph() {
		for (auto i: E) delete i.second;
		for (auto i: V) delete i.second;
	}

	struct visitor {
		enum algorithm {
			breath_first, depth_first
		
		};
		virtual void start(const vertex&)=0;
		virtual void visit(const edge&)=0;
		virtual void finished()=0;
	};
	void breath_first(int iv, visitor& visitor_) const {
		auto v=V.find(iv);
		if (v==V.end()) return;
		breath_first(*v->second,visitor_);
	}

	void breath_first(const vertex& s, visitor& visitor_) const {
		visitor_.start(s);
		unordered_set<edge*> visited;
		queue<edge*> q;
		for (auto e:s.e) {
			q.push(e);
		}
		while(!q.empty()) {
			edge* e=q.front(); q.pop();
			if (visited.find(e)==visited.end()) {		
				for (auto ed:e->to->e) {
					q.push(ed);
				}
				visitor_.visit(*e);
				visited.emplace(e);
			}
		}
		visitor_.finished();
	}

	void depth_first(int iv, visitor& visitor_) const {
		auto v=V.find(iv);
		if (v==V.end()) return;
		depth_first(*v->second,visitor_);
	}

	void depth_first(const vertex& s, visitor& visitor_) const {
		visitor_.start(s);
		unordered_set<edge*> visited;

		stack<edge*> q;
		for (auto e:s.e) {
			q.push(e);
		}
		while(!q.empty()) {
			edge* e=q.top(); q.pop();
			if (visited.find(e)==visited.end()) {		
				for (auto ed:e->to->e) {
					q.push(ed);
				}
				visitor_.visit(*e);
				visited.emplace(e);
			}
		}
		visitor_.finished();
	}

	
};



struct vis:graph::visitor {
	void start(const vertex&) override {};
	void visit(const edge& e) override {
		cout << (e.from!=0?e.from->id:-1) << " -> " << e.to->id << endl;
	}
	void finished() override {};
};


template<typename T, typename data>
struct best_path:graph::visitor {
	unordered_map<const edge*,T> values;



	unordered_map<const vertex*,data> vd; //vertex data
	const graph& g;
	const vertex* f{0};
	const vertex* t{0};
	struct result:vector<const vertex*> {
		void dump(ostream& os) const {
			for (auto i:*this) os << i->id << endl;
		}
	};
	result r;
	best_path(const graph&g_, const vector<string>& al_values): g(g_) { //adjacency list + values
		for (auto& l:al_values) {
			int v1=-1;
			int v2=-1;
			istringstream is(l);
			is >> v1;
			is >> v2;
			ostringstream os;
			os << v1 << " " << v2;
			auto ie=g.E.find(os.str());
			if (ie==g.E.end()) {
				cerr << "edge not found " << os.str() << endl;
				continue;
			}
			string value;
			getline(is,value);
			T t(value);
			values.emplace(ie->second,move(t));
		}
	}
	~best_path() {
	}

	result compute(int from, int to,algorithm a) {
		auto ifrom=g.V.find(from);
		if (ifrom==g.V.end()) {
			cerr << from << " node not found" << endl;
			return r;
		}
		else f=ifrom->second;

		auto ito=g.V.find(to);
		if (ito==g.V.end()) {
			cerr << to << " node not found" << endl;
			return r;
		}
		else t=ito->second;
		switch(a) {
			case breath_first:
				g.breath_first(*f,*this);
				break;
			case depth_first:
				g.depth_first(*f,*this);
				break;
			default: cerr << "unknown algorithm" << endl;
		}
		return r;
	}

	data& relax(const edge& e) {
		auto& ed=values.find(&e)->second;
		auto ivdf=vd.find(e.from);
		if (ivdf==vd.end()) {
			vd.emplace(e.from,data());
			ivdf=vd.find(e.from);
		}
		auto ivdt=vd.find(e.to);
		if (ivdt==vd.end()) {
			vd.emplace(e.to,data());
			ivdt=vd.find(e.to);
		}
		data& df=ivdf->second;
		data& dt=ivdt->second;
		auto sum=ed+df;	
		if (sum<dt) {
			dt=sum;
			dt.from=&e;
		}
		return dt;
	}


	void start(const vertex& s) override {
		r.clear();
		vd.clear();
		vd.emplace(&s,data(0));
	};
	
	void visit(const edge& e) override {
		relax(e);
/*
		if (e->to==t) { //he llegado a f
			if (best!=0) {
				if (d<*best) {
					best=&d;
				}
			}
			else {
				best=&d;
			}
		}
*/
	}

	void finished() override {
		auto icur=vd.find(t);
		if (icur==vd.end()) return; //no hay conexion con el nodo buscado
		data*cur=&icur->second;
		vector<const vertex*> rp;
		while(cur->from!=0) {
			rp.push_back(cur->get_dst_vertex());
			const vertex* src=cur->get_src_vertex();
			if (src==0) break;
			cur=&vd.find(src)->second;
		}		
		//reverse it
		r.reserve(rp.size()+1);
		for (auto i=rp.rbegin(); i!=rp.rend(); ++i) {
			r.push_back(*i);
		}
		r.push_back(f);
	}


};


struct data {
	data():score(numeric_limits<int>::max()) {}
	data(int v):score(v) {}
	const vertex* get_dst_vertex() const { return from->to; }
	const vertex* get_src_vertex() const { return from->from; }
	int score;
	const edge* from{0};
	bool operator <(const data& other) const { return score<other.score; }
};

template<typename T>
struct scalar {
	T value{0};
	scalar(string& s) {
		istringstream is(s);
		is>>value;
	}
	data operator+(const data& d) const {
		data r;
		r.score=d.score+value;
		return move(r);
	}	
};

struct order;
///-------------------------------------------------------------------------------------------------
struct drone {
	drone(int id_):id(id_) {
cout << "created drone " << id  << endl;
	}

	static int capacity;
	int r{0};
	int c{0};

	int tr{0};
	int tc{0};

	int id{-1};

	int status{0};    ///0 - wait
				// 1.- travelling to load
				// 2.- loaded
				// 3.- travelling to deliver payload

	int transit{0};

	map<order*,map<int,int>> payload; //product type, units
	multimap<double,order*> orders; //destinations

	bool available() const {
		return status==0;
	}

	void deliver();

	void tick(int ticks_left) {
		cout << "drone " << id << " tick, arrive in " << transit << "; status " << status << endl;
		if (transit>0) --transit;
		if (transit>0) return;
		if (status==1) {
			cout << "drone arrived warehouse" << endl;

			status=2;
		}
		if (status==2) {
			//trazar la ruta y volar
			cout << "computing route to deliver" << endl;
			if (orders.empty()) {
				status==0; //idle
			}
			else {
				deliver();
			}
		}
	}
	

	bool callable() {
		if (status!=0) return false;
		return true;
	}


};

int drone::capacity=0;

struct drones: vector<drone> {
	drone* call(int r, int c) {  ///tiempo de espera
		
		map<double,drone*> dr;
		for (auto& i:*this) {
			if (i.callable()) {
				double d=sqrt(pow((double)i.r-(double)r,2)+pow((double)i.c-(double)c,2));

				dr.emplace(d,&i);
			}						
		}
		if (dr.empty()) return 0;
		drone* d=dr.begin()->second;
		cout << "called drone " << d->id << endl;
		return d;
	}
	bool available() const {
		for (auto& i:*this) if (i.available()) return true;
	}

	void tick(int tleft) {
		cout << "drones tick" << endl;
		for (auto& i:*this) i.tick(tleft);
	}
};

drones _drones;


struct product_type {
	int id;
	int weight;
};

struct product_types:vector<product_type> {
};

product_types pts;

struct orders;
struct order;

struct warehouse:vector<int> {
	warehouse(int id_):id(id_) {
		reserve(pts.size());
		for (int i=0; i<pts.size(); ++i) emplace_back(0);
	}
	int id{0};
	int r{0};
	int c{0};

	multimap<double,order*> _orders;

	void set_orders(orders&);

	double get_nearest_order_distance() const {
		if (_orders.empty()) return numeric_limits<double>::max();
		return _orders.begin()->first;
	}

	int get_from_stock(int pt, int items) {
		if ((*this)[pt]<items) items=(*this)[pt];
		(*this)[pt]-=items;
		return items;
	}

	void dispatch();
	
};
struct warehouses:vector<warehouse>{
	void set_orders(orders& o) {
		for (auto& i:*this) {
			i.set_orders(o);
//			cout << "wh" << i.id << " size " << i._orders.size() << endl;

		}
//		cout << "wh0 size " << (*this)[0]._orders.size() << endl;
	}

	void dispatch() {
		while(_drones.available()) {
			double d=numeric_limits<double>::max();
			warehouse* next=0;
			for (auto& i:*this) {
				double dw=i.get_nearest_order_distance();
				if (dw<d) {
					d=dw;
					next=&i;
				}
			}
			if (next==0) break;
			next->dispatch();
		}
	}
	

};

warehouses wrs;

struct order:map<int,pair<int,int>> {  //product type, items requested, items delivered(in transit)
	int id{0};
	int r{0};
	int c{0};

	void delivered(int pt,int am) {
		auto i=find(pt);
		i->second.second-=am;
		if (i->second.second==0) {
			if (i->second.first==0) {
				erase(i);
			}
		}
	}

	int load_items(warehouse& w, map<order*,map<int,int>> items, int space_left) {
		//items pt,amount
		for (auto& i:*this) {
			int am=i.second.first-i.second.second;
			if (space_left<am) am=space_left;
			int a=w.get_from_stock(i.first,am); ///requested-delivered
			if (a>0) {
				auto j=items.find(this);
				if (j==items.end()) {
					map<int,int> h;
					h.emplace(i.first,0);
					items.emplace(this,move(h));
					j=items.find(this);
				}
				auto m=j->second.find(i.first);
				if (m==j->second.end()) {
					j->second.emplace(i.first,0);
					m=j->second.find(i.first);
				}
				m->second+=a;
				i.second.second+=a; //delivered
			}
			if (space_left==0) break;
		}
		
		return space_left;
		
	}

	void add(int pt) {
		auto i=find(pt);
		if (i==end()) {
			emplace(pt,pair<int,int>(0,0));
			i=find(pt);
		}
		++i->second.first;
	}
	
};

struct orders:vector<order> {

	bool finished() const {
		bool b=true;
		for (auto& i:*this) {
			if (!i.empty()) {
				b=false;
				break;
			}
		}
		return b;
	}
};

orders o;




struct command {
	int drone{-1};
	char op;  // L U  load/unload
	int warehouse;
	int pt;
	int n;
	void dump(ostream&os) const {
	    os << drone << " " << op << " " << warehouse << ' ' << pt << ' ' << n << endl;
	}
};

struct commands: vector<command*> {
	void dump(ostream&os) const {
	    os << size() << endl;
	    for (auto i:*this) {
		i->dump(os);
	    }
	}
	
};

commands cmds;



void warehouse::set_orders(orders& o) {
	_orders.clear();
	for (auto& i:o) {
		double d=sqrt(pow((double)i.r,2)+pow((double)i.c,2));

		//double n=floor(d);
		//if (d<n) d=n+1;
		_orders.emplace(d,&i);
	}
	cout << "warehouse " << id << " order size: " << _orders.size() << " " << o.size() << " " << &_orders << endl;
}

void warehouse::dispatch() {
	cout << "warehouse " << id <<" dispatch"<< endl;
	if (_orders.empty()) {
		cout << "warehouse " << id <<": no orders to dispatch " << &_orders << endl;
		return;
	}
	//pedir 1 dron
	drone* drn=_drones.call(r,c);
	if (drn==0) {
		cout << "no drones available" << endl;
		return;
	}
cout << "AAdrn " << drn->id << endl;
	//obtener entre los pedidos las ordenes mas cercanas hasta agotar capacidad.
	int space_left=drone::capacity;
	map<order*,map<int,int>> items; //type, amount
	multimap<double,order*> ords;
	for (auto& i:_orders) {
		int nspace_left=i.second->load_items(*this,items,space_left);
		if (nspace_left<space_left) {
			ords.emplace(i.first,i.second);
			space_left=nspace_left;
		}
		if (space_left==0) break;
	}
	drn->payload=move(items);
	drn->orders=move(ords);
	if (drn->r==r && drn->c==c) {
		cout << "drone " << drn->id << " is here. loaded " << endl;
		drn->status=2;  //loaded
	}
	else {
		if (drn->status==0) { //idle
			drn->status=1; //coming to load
			double d=sqrt(pow((double)r-(double)drn->r,2)+pow((double)c-(double)drn->c,2));

			double d0=floor(d);
			if (d0<d) d=d0+1;

			drn->transit=(int)d;
			map<int,int> col;
			for (auto& i:items) {
			for (auto& j:i.second) {
				auto g=col.find(j.first);
				if (g==col.end()) {
					col.emplace(j.first,0);
					g=col.find(j.first);
				}
				g->second+=j.second;
			}
			}
			for (auto& i:col) {
				command* cmd=new command();
				cmd->drone=drn->id;
				cmd->warehouse=id;
				cmd->op='L';
				cmd->pt=i.first;
				cmd->n=i.second;
				cmds.push_back(cmd);
				cout << "added command Load drone " << drn->id << endl;
			}

		}
		cout << "drone " << drn->id << " is coming in " << drn->transit << endl;
	}



	

}


void drone::deliver() {
	auto i=orders.begin();
	order* ord=i->second;
	double d=i->first;
	double d0=floor(d);
	if (d0<d) d=d0+1;
	transit=d;

	auto mit=payload.find(ord);
	map<int,int>& m=mit->second;

	for (auto& p:m) {
		command* cmd=new command();
		cmd->drone=id;
		cmd->warehouse=ord->id; //customer
		cmd->op='D';
		cmd->pt=p.first;
		cmd->n=p.second;
		cmds.push_back(cmd);
		cout << "added command Deliver drone " << id << " to client " << ord->id << endl;
		ord->delivered(p.first,p.second);
	}
	payload.erase(mit);
	orders.erase(i);
	status=3;
}

///-----------------------------------------------------------------------

#include <sstream>
#include <fstream>
int main() {

int NR=0;
int NC=0;
int D=0;
int turns=0;
{//400 600 30 112993 200
string header;
getline(cin,header);
istringstream is(header);
is >> NR;
is >> NC;
is >> D;
is >> turns;
is >> drone::capacity;
}

_drones.reserve(D);
for (int i=0; i<D; ++i) {
	_drones.push_back(drone(i));
}

int P=0;
{//400 600 30 112993 200
{
string header;
getline(cin,header);
istringstream is(header);
is >> P;
}
string line;
getline(cin,line);
istringstream is(line);
//73 40 84 107 52 36 13 74 36 94 93 46 123 24 100 93 62 49 97 102 80 37 22 25 72 48 40 74 32 31 136 64 99 37 44 36 104 74 112 40 65 67 50 143 23 26 91 20 142 128 9 77 40 26 55 104 59 112 42 69 87 89 2 11 105 43 105 23 21 88 57 40 52 63 35 141 54 27 45 37 21 37 102 38 36 117 57 93 24 52 83 133 51 102 34 52 39 54 64 146 127 100 58 50 49 40 55 123 64 30 127 63 19 59 31 22 68 37 107 68 52 61 35 38 76 44 21 144 21 40 105 48 97 70 74 95 75 28 25 40 81 72 58 99 52 5 72 84 53 21 110 97 30 43 92 109 39 89 51 30 102 94 13 45 93 94 90 104 36 55 92 12 45 126 73 132 118 127 125 26 83 69 49 39 4 63 84 47 75 43 82 43 51 28 118 110 42 2 53 74 54 57 92 23 72 95 76 35 38 100 52 34 58 74 86 25 32 61 39 55 76 68 107 82 97 108 57 38 88 71 22 40 63 37 80 51 52 52 67 101 49 46 100 52 63 36 53 45 51 56 71 50 13 29 18 36 87 86 39 11 148 74 74 121 11 28 65 79 32 85 95 26 43 123 81 78 25 100 27 90 53 60 100 25 72 80 83 40 62 53 40 97 105 56 49 100 36 46 80 8 23 58 78 58 24 78 67 62 85 80 39 34 62 33 58 130 41 34 33 62 129 79 103 104 56 33 118 96 21 18 65 140 87 91 61 54 137 71 84 35 75 32 4 68 37 80 78 91 75 52 74 96 32 85 42 78 119 58 16 44 24 98 121 76 16 56 112 67 58 46 76 45 41 94 55 44 51 136 63 34 86 87 64 54 27 69 31 64 138 56 97 81 40 132 64 114 105 41 52 60
int id=0;
while(!is.eof()) {
	product_type pt;
	pt.id=id;
	is >> pt.weight;
	pts.emplace_back(move(pt));
}
}

int W=0;
{
{
string header;
getline(cin,header);
istringstream is(header);
is >> W;
}
string line;
getline(cin,line);
istringstream is(line);
//73 40 84 107 52 36 13 74 36 94 93 46 123 24 100 93 62 49 97 102 80 37 22 25 72 48 40 74 32 31 136 64 99 37 44 36 104 74 112 40 65 67 50 143 23 26 91 20 142 128 9 77 40 26 55 104 59 112 42 69 87 89 2 11 105 43 105 23 21 88 57 40 52 63 35 141 54 27 45 37 21 37 102 38 36 117 57 93 24 52 83 133 51 102 34 52 39 54 64 146 127 100 58 50 49 40 55 123 64 30 127 63 19 59 31 22 68 37 107 68 52 61 35 38 76 44 21 144 21 40 105 48 97 70 74 95 75 28 25 40 81 72 58 99 52 5 72 84 53 21 110 97 30 43 92 109 39 89 51 30 102 94 13 45 93 94 90 104 36 55 92 12 45 126 73 132 118 127 125 26 83 69 49 39 4 63 84 47 75 43 82 43 51 28 118 110 42 2 53 74 54 57 92 23 72 95 76 35 38 100 52 34 58 74 86 25 32 61 39 55 76 68 107 82 97 108 57 38 88 71 22 40 63 37 80 51 52 52 67 101 49 46 100 52 63 36 53 45 51 56 71 50 13 29 18 36 87 86 39 11 148 74 74 121 11 28 65 79 32 85 95 26 43 123 81 78 25 100 27 90 53 60 100 25 72 80 83 40 62 53 40 97 105 56 49 100 36 46 80 8 23 58 78 58 24 78 67 62 85 80 39 34 62 33 58 130 41 34 33 62 129 79 103 104 56 33 118 96 21 18 65 140 87 91 61 54 137 71 84 35 75 32 4 68 37 80 78 91 75 52 74 96 32 85 42 78 119 58 16 44 24 98 121 76 16 56 112 67 58 46 76 45 41 94 55 44 51 136 63 34 86 87 64 54 27 69 31 64 138 56 97 81 40 132 64 114 105 41 52 60
int id=0;
wrs.reserve(W);
for(int i=0; i<W; ++i) {
	warehouse w(i);
	w.id=i;
	{
	string line;
	getline(cin,line);
	istringstream is(line);
	is >> w.r;
	is >> w.c;
	if (i==0) {
		for (auto d:_drones) {
			d.r=w.r;
			d.c=w.c;
		}
	}
	}
	{
	string line;
	getline(cin,line);
	istringstream is(line);
	for (int i=0; i<P; ++i) {
		int n;
		is >> n;
		w[i]=n;
	}
	}
	wrs.push_back(w);
}
}
int C=0;
{
string line;
getline(cin,line);
istringstream is(line);
is >> C;
cout << C << " orders" << endl ;
int id=0;
o.reserve(C);
for(int i=0; i<C; ++i) {
	order ord;
	ord.id=i;
	{
	string line;
	getline(cin,line);
	istringstream is(line);
	is >> ord.r;
	is >> ord.c;
	}
	int L=0;
	{
	string line;
	getline(cin,line);
	istringstream is(line);
	is >> L;
	}
	{
	string line;
	getline(cin,line);
	istringstream is(line);
	for (int i=0; i<L; ++i) {
		int n;
		is >> n;
		ord.add(n);
	}
	}
	o.emplace_back(move(ord));
}

}

for (auto& i: _drones) {
	i.r=wrs[0].r;
	i.c=wrs[0].c;
}


wrs.set_orders(o);

cout << "o size " << o.size() << endl;

for (int t=0; t<turns; ++t) {
	cout << endl;
	cout << "time " << t << " orders left " << o.size() << endl;
	if (o.empty()) break;
	wrs.dispatch();
	_drones.tick(turns-t);
	if (o.finished()) {
		cout << "finished, no items left to deliver" << endl;
		break;
	}
}






//orders.plan();

cmds.dump(cout);

return 0;

}


