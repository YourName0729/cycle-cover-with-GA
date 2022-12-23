
#pragma once





class ESdata {
public :
	ESdata() {;} 
	ESdata(unsigned T, unsigned t, float best_ratio, float best_solv4, float best_GAs ) : T(T), t(t), best_ratio(best_ratio), best_solv4(best_solv4), best_GAs(best_GAs) {}
	
    unsigned T, t ;
    float best_ratio, best_solv4, best_GAs ;
};


class ESstats {
public :

	std::vector<ESdata> infos ;
};

