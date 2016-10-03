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
#include <iostream>
#include <string>
#include <map>

using namespace std;

struct canvas;

struct rect {
	int l,t,r,b;

	bool intersects(const rect& other) const {
		if (r<other.l) return false;
		if (l>other.r) return false;
		if (t>other.b) return false;
		if (b<other.t) return false;
		return true;
	}

//	virtual void grow()=0;
};


struct command {
	//unsigned int cost{1};
	command* _next{0};
	int score{0};

	command();
	~command();

	int length() const;
	virtual	int area() const=0;
	virtual	rect get_rect() const=0;
	virtual void paint(canvas& src, const canvas&tgt);
	virtual void dump(ostream& os) const;
	bool set_next(command* next);
	static command* select(command* c1, command* c2);

	bool update(const canvas& src,const canvas& dst,const rect&r);
};


struct cache: multimap<int,command*> {
	canvas *src{0};
	canvas *tgt{0};
	vector<command*> seq;

	int mode{0};

	int best_score{0};
	void set_mode1() {
		mode=1;
		best_score=0;
	}

	void dump() const {
		cout << "cache" << endl;
		for (auto i=begin(); i!=end(); ++i) {
			cout << i->first << endl;
		}
		cout << "end of cache" << endl;
	}

	void commit(int limit_score);
	void update(const canvas& src, const canvas& tgt,const rect& r);
	void add(command* cmd);

};
cache _cache;

struct canvas;

struct canvas: vector<string> {

	int offsetx{0};
	int offsety{0};

	void dump(int indent) const {
		for (auto s:*this) cout << string(indent,' ') << s << endl;
	}
	void dump(ostream& os) const {
		for (auto s:*this) os << s << endl;
	}
	int width() const {
		return (*this)[0].size();
	}
	int height() const {
		return (*this).size();
	}

	canvas cut(const rect& r) const {
		int x0=r.l-offsetx;		
		int x1=r.r-offsetx+1;		
		int y0=r.t-offsety;		
		int y1=r.b-offsety+1;		
		if (x0<0) x0=0;
		if (y0<0) y0=0;
		if (x1>width()) x1=width();
		if (y1>height()) y1=height();
		canvas c;
		c.reserve(y1-y0);
		for (int i=y0; i<y1; ++i) {
			c.emplace_back((*this)[i].substr(x0,x1-x0));
		}
		c.offsetx=x0+offsetx;
		c.offsety=y0+offsety;
		return move(c);
	}

	struct square_cmd: command {

		int _x{-1}, _y{-1}, _s{0};

		int area() const {
			return (1+2*_s)*(1+2*_s);
		}

		void dump(ostream& os) const {
			os << "PAINT_SQUARE " << _y << " " << _x << " " << _s << endl;
			command::dump(os);
		}

		virtual rect get_rect() const {
			rect r;
			r.l=_x-_s;
			r.r=_x+_s;
			r.t=_y-_s;
			r.b=_y+_s;
			return move(r);
		}
		

		virtual void paint(canvas& c, const canvas& target) {
			int x0=_x-_s;
			int x1=_x+_s+1;
			int y0=_y-_s;
			int y1=_y+_s+1;
			score=0;
			for (int y=y0; y<y1; ++y) {
				auto& row=c[y-c.offsety];
				auto& rowt=target[y-target.offsety];
				for (int x=x0; x<x1; ++x) {
					char& ch=row[x-c.offsetx];
					if (ch=='.') {
						if (rowt[x-target.offsetx]=='#') --score; //best scores are negative
						else ++score;
						ch='#';				
					}
				}
			}
			command::paint(c,target);
		}

		static void grow(rect& r) {
			int w=r.r-r.l;
			r.l-=w;
			r.r+=w;
			int h=r.b-r.t;
			r.t-=h;
			r.b+=h;
		}

		static bool fill_cache(int S,const canvas& src, const canvas& target) {
		int ret=false;
		int incr=1;
		if (src.width()<incr) return ret;
		if (src.height()<incr) return ret; //canvas too small

		int x0=S;
		int x1=target.width()-S;
		int y0=S;
		int y1=target.height()-S;
		square_cmd* curcmd=0;
		square_cmd* cmd=new square_cmd();
		cmd->_s=S;
		int y=y0-incr;
		while (true) {
			y+=incr;
			if (y==y1) break;
			if (y>y1) {
				if ((y-incr)==(y1-1)) break; //we've done this case already, exit inf loop
				y=y1-incr-1;
				if (y<S) break;
			}
			int x=x0-incr;
			while (true) {
				x+=incr;
				if (x==x1) break;
				if (x>x1) {
					if ((x-incr)==(x1-1)) break; //we've done this case already, exit inf loop
					x=x1-incr-1;	
					if (x<S) break;
				}
				cmd->_x=x+target.offsetx;
				cmd->_y=y+target.offsety;
				canvas tile=src.cut(cmd->get_rect());
				cmd->paint(tile,target);
				if (cmd->score<=((-cmd->area()))) {
					_cache.add(cmd);

					cout << "SQ cache size " << _cache.size() << " score " << cmd->score << endl;
					ret=true;
				}
				else {
					delete cmd;
				}
				cmd=new square_cmd();
				cmd->_s=S;
			}
		}
		return ret;
		}
	};

	struct line_cmd: command {
		int _x0{-1}, _y0{-1}, _x1{-1}, _y1{-1};

		int area() const {
			return _y1-_y0+_x1-_x0;
		}
		virtual rect get_rect() const {
			rect r;
			r.l=_x0;
			r.r=_x1;
			r.t=_y0;
			r.b=_y1;
			return move(r);
		}

		void dump(ostream& os) const {
			os << "PAINT_LINE " << _y0 << " " << _x0 <<  " " << _y1 <<  " " << _x1 << endl;
			command::dump(os);
		}

		virtual void paint(canvas& c,const canvas& target) {
			score=0;
			if (_x0==_x1) {
				int cx0=_x0-c.offsetx;
				int tx0=_x0-target.offsetx;
				for (int y=_y0; y<_y1+1; ++y) {
					char& ch=c[y-c.offsety][cx0];
					if (ch=='.') {
						if (target[y-target.offsety][tx0]=='#') --score;
						else ++score;
						ch='#';				
					}
				}
			}
			else {
				auto& row=c[_y0-c.offsety];
				auto& rowt=target[_y0-target.offsety];
				for (int x=_x0; x<(_x1+1); ++x) {
					char& ch=row[x-c.offsetx];
					if (ch=='.') {
						if (rowt[x-target.offsetx]=='#') --score;
						else ++score;
						ch='#';				
					 }
				}
			}
			command::paint(c,target);
		}
		static void fill_cache2V(int L,const canvas& src, const canvas& target) {
			int incr=1;
			if (src.height()<incr) return;

			int x0=0;
			int x1=target.width();
			int y0=0;
			int y1=target.height()-L;
			line_cmd* curcmd=0;
			line_cmd* cmd=new line_cmd();
			int y=y0-incr;
			while (true) {
				y+=incr;
				if (y==y1) break;
				if (y>y1) {
					if ((y-incr)==(y1-1)) break; //we've done this case already, exit inf loop
					y=y1-incr-1;	
					if (y<0) break;
				}

				for (int x=x0; x<x1; ++x) {
					cmd->_x0=x+target.offsetx;
					cmd->_y0=y+target.offsety;
					cmd->_x1=cmd->_x0;
					cmd->_y1=cmd->_y0+L;

					canvas tile=src.cut(cmd->get_rect());
					cmd->paint(tile,target);
/*
tile.dump(0);
canvas tiletgt=target.cut(cmd->get_rect());
tiletgt.dump(0);
cout << "score " << cmd->score << endl << endl;;
*/

					if (cmd->score<=((-cmd->area()))) {
						_cache.add(cmd);

						cout << "LV cache size " << _cache.size() << " score " << cmd->score << endl;
					}
					else {
						delete cmd;
					}
					cmd=new line_cmd();
				}
			}
		}

		static void fill_cache2H(int L,const canvas& src, const canvas& target) {
//			int incr=L; //fast search
			int incr=1; //fast search
			if (src.width()<incr) return;

			int x0=0;
			int x1=target.width()-L;
			int y0=0;
			int y1=target.height();
			line_cmd* curcmd=0;
			line_cmd* cmd=new line_cmd();
			int maxscore=L;
			for (int y=y0; y<y1; ++y) {
				int x=x0-incr;
				while (true) {
					x+=incr;
					if (x==x1) break;
					if (x>x1) {
						if ((x-incr)==(x1-1)) break; //we've done this case already, exit inf loop
						x=x1-incr-1;	
						if (x<0) break;
					}
					cmd->_x0=x+target.offsetx;
					cmd->_y0=y+target.offsety;
					cmd->_x1=cmd->_x0+L;
					cmd->_y1=cmd->_y0;

					canvas tile=src.cut(cmd->get_rect());
					cmd->paint(tile,target);
/*
tile.dump(0);
canvas tiletgt=target.cut(cmd->get_rect());
tiletgt.dump(0);
cout << "score " << cmd->score << endl << endl;;
*/

					if (cmd->score<=((-cmd->area()))) {
						_cache.add(cmd);
						cout << "LH cache size " << _cache.size() << " score " << cmd->score << endl;
					}
					else {
						delete cmd;
					}
					cmd=new line_cmd();
				}
			}
		}
		static void growV(rect& r) {
			int h=r.b-r.t;
			r.t-=h;
			r.b+=h;
		}
		static void growH(rect& r) {
			int w=r.r-r.l;
			r.l-=w;
			r.r+=w;
		}


		static void fill_cacheH(int length,const canvas& src, const canvas& target) {
			int maxlength=target.height(); //
			if (length>maxlength) return;
			fill_cache2H(length,src,target);
		}
		static void fill_cacheV(int length,const canvas& src, const canvas& target) {
			int maxlength=target.height(); //
			if (length>maxlength) return;
			fill_cache2V(length,src,target);
		}

		static void fill_cache(int length, const canvas& src, const canvas& target) {
			fill_cacheH(length,src,target);
			fill_cacheV(length,src,target);
		}
	};
	struct erase_cmd: command {
		int _x{-1};
		int _y{-1};
		int area() const { return 1; }

		void dump(ostream& os) const {
			os << "ERASE_CELL " << _y << " " << _x << endl;
			command::dump(os);
		}
		virtual rect get_rect() const {
			rect r;
			r.l=_x;
			r.r=_x;
			r.t=_y;
			r.b=_y;
			return move(r);
		}

		virtual void paint(canvas& c, const canvas& target) {
			score=0;
			if (c[_y-c.offsety][_x-c.offsetx]=='#') {
				if (target[_y-target.offsety][_x-target.offsetx]=='.') --score;
				else ++score;
				c[_y-c.offsety][_x-c.offsetx]='.';				
			}
			command::paint(c,target);
		}

		static void collect_erase(vector<erase_cmd*>& v, const canvas& src, const canvas& target) {
//			cout << "starting coords " << x0 << " " << y0 << endl;
			int x1=target.width();
			int y1=target.height();
			erase_cmd* cmd=new erase_cmd();
			for (int y=0; y<y1; ++y) {
				auto& trow=target[y];
				auto& srow=src[y];
				for (int x=0; x<x1; ++x) {
					if (trow[x]=='.' && srow[x]!='.') {
						cmd->_x=x+target.offsetx;
						cmd->_y=y+target.offsety;
						cmd->score=-1;
/*
canvas copy=src;
cmd->paint(copy,'@');
copy.dump(2);
target.dump(4);
*/
						v.push_back(cmd);
						cmd=new erase_cmd();
					}
				}
			}
			delete cmd;
		}
	};
	canvas wipe() {
		canvas c;
		string s((*this)[0].size(),'.');
		c.reserve(size());
		for (int i=0; i<size(); ++i)
			c.push_back(s);
		c.offsetx=offsetx;
		c.offsety=offsety;
		return move(c);
		
	}

	void fill_cache(const canvas& target) const {
		int ms=1+min(width(),height())/2;
		ms-=60;
		int mlH=width();
		int mlV=height();
		int stop=max(mlH,mlV);
		int s=ms;
		int lh=mlH;
		int lv=mlV;

		while (true) {
			_cache.commit(0); //comitear todos los mejores al mejor de los que van a venir
			int sqA=(2*s+1)*(2*s+1);
			int lhA=lh;
			int lvA=lv;
			if (sqA>max(lhA,lvA)) {
				if (s>=0) {
					cout << "SQ " << s << endl;
					square_cmd::fill_cache(s,*this,target);
				}
				--s;
				continue;
			}
			else if (lvA>lhA) {
				if (lv>=1) {
					cout << "LV " << lv << endl;
					if (lv==0) break;
					line_cmd::fill_cacheV(lv,*this,target);
				}
				--lv;
			}
			else {
				if (lh>=1) {
					cout << "LH " << lh << endl;
					line_cmd::fill_cacheH(lh,*this,target);
				}
				--lh;
			}
			if (s<0 && lh<0 && lv<0) break;
	
		}
		_cache.commit(0); //comitear todos los mejores al mejor de los que van a venir
	}


};

command::command() {
}

command::~command() {
	delete _next;
}

void command::paint(canvas& c,const canvas& tgt) {
	if (_next!=0) _next->paint(c,tgt);
}
int command::length() const {
	if (_next==0) return 1;
	return 1+_next->length();
}
void command::dump(ostream& os) const {
	if (_next!=0) _next->dump(os);
}

bool command::set_next(command* next) {
	delete _next;
	_next=next;
	
}
bool command::update(const canvas& src,const canvas& target,const rect&r) {
	rect mr=get_rect();
	if (!mr.intersects(r)) return false;
	int prevscore=score;
	canvas tile=src.cut(mr);
	paint(tile,target);
	return score!=prevscore;
}
void cache::add(command* cmd) {
	if (mode==0) {
		emplace(cmd->score,cmd);
	}
	else {
		if (cmd->score<best_score)
			best_score=cmd->score;
		delete cmd;
	}
}

void cache::update(const canvas& src, const canvas& tgt,const rect& r) {
	vector<command*> updated;
	for (auto i=begin(); i!=end(); ) {
		if (i->second->update(src,tgt,r)) {
			updated.push_back(i->second);
			iterator j=i;
			++i;
			erase(j);
		}
		else {
			++i;
		}
	}
	for (auto i:updated) {
		if (i->score<-i->area()/2) {
			emplace(i->score,i);
		}
		else {
			delete i;
		}
	}
}
void cache::commit(int limit_score) {
	cout << "committing scores better than " << limit_score << endl;
	while(!empty()) {
		cout << "cache size " << size() << endl;
		command* cmd=begin()->second;
		if (cmd->score>limit_score) break;
		erase(begin());
		cmd->paint(*src,*tgt);
		update(*src,*tgt,cmd->get_rect());
		seq.push_back(cmd);
	}
	mode=0;
}

#include <sstream>
#include <fstream>
int main() {

string header;
getline(cin,header);
int w=0;
int h=0;
istringstream is(header);
is >> w;
is >> h;
//cout << w << " " << h << endl;
if (w<=0) return 1;
if (h<=0) return 1;
canvas target;
target.reserve(h);
{
string line;
getline(cin,line);
w=line.size();
target.emplace_back(move(line));
}
while(!cin.eof()) {
	string line;
	getline(cin,line);
	if (line.size()!=w) continue;
	target.emplace_back(move(line));
}

//target.dump(0);
canvas src=target.wipe();
//src.fill_cache(target);

_cache.src=&src;
_cache.tgt=&target;

src.fill_cache(target);

_cache.commit(0); //flush the cache


cout << "collecting erase commands" << endl;
vector<canvas::erase_cmd*> v;
canvas::erase_cmd::collect_erase(v, src,  target);
cout << "size " << v.size() << endl;
for (auto i:v) {
	i->paint(src,target);
}

cout << "original:" << endl;
target.dump(10);
cout << endl;
cout << "generated:" << endl;
src.dump(0);
cout << endl;

{
ofstream ofs("commands.out");
ofs << _cache.seq.size()+v.size() << endl;
for (auto i:_cache.seq) {
	i->dump(ofs);
}
for (auto i:v) {
	i->dump(ofs);
}
}


{
ofstream ofs("paint.out");
src.dump(ofs);
}


exit(0);
return 0;

}


