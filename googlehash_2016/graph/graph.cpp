#include <vector>
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

int main() {
	vector<string> al;
	al.push_back("1 2");
	al.push_back("1 3");
	al.push_back("1 4");
	al.push_back("2 5");
	al.push_back("2 6");
	al.push_back("3 7");
	al.push_back("4 8");
	al.push_back("4 9");
	al.push_back("5 10");
	al.push_back("5 11");
	al.push_back("6 12");
	al.push_back("7 13");
	al.push_back("8 14");
	al.push_back("14 8");
	al.push_back("9 2");
	al.push_back("6 2");
	al.push_back("12 3");
	
	graph g(al);
	g.dot(cout);

	vis vis_;
	cout << "breath first traversal" << endl;
	g.breath_first(1,vis_);
	cout << "depth first traversal" << endl;
	g.depth_first(1,vis_);

	cout << "best path" << endl;
	vector<string> distances;
	distances.push_back("1 2 1");
	distances.push_back("1 3 1");
	distances.push_back("1 4 1");
	distances.push_back("2 5 1");
	distances.push_back("2 6 1");
	distances.push_back("3 7 1");
	distances.push_back("4 8 1");
	distances.push_back("4 9 1");
	distances.push_back("5 10 1");
	distances.push_back("5 11 1");
	distances.push_back("6 12 1");
	distances.push_back("7 13 1");
	distances.push_back("8 14 1");
	distances.push_back("14 8 1");
	distances.push_back("9 2 1");
	distances.push_back("6 2 1");
	distances.push_back("12 3 1");

	typedef best_path<scalar<int>,data> pathfinder;
	pathfinder bp(g,distances);
	{
	cout << "best path from 1 to 11 )" << endl;
	auto r=bp.compute(1,11,pathfinder::breath_first);
	r.dump(cout);
	}
	{
	cout << "best path from 14 to 2 )" << endl;
	auto r=bp.compute(14,2,pathfinder::breath_first);
	r.dump(cout);
	}
	{
	cout << "best path from 1 to 2 )" << endl;
	auto r=bp.compute(1,2,pathfinder::depth_first);
	r.dump(cout);
	}


	return 0;
}

