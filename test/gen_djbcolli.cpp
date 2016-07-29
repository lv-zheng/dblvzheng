#include <iostream>

char colli[2][5] = {"GbGb", "HAHA"};

#define rep(i, n) for (int i = 0; i < n; ++i)

int main()
{
	rep(i, 2)
	rep(j, 2)
	rep(k, 2)
	rep(l, 2)
	rep(m, 2)
	rep(n, 2)
	rep(o, 2)
	rep(p, 2)
	rep(q, 2)
	rep(r, 2)
	rep(s, 2)
	rep(t, 2)
		std::cout
			<< colli[i] << colli[j]
			<< colli[k] << colli[l]
			<< colli[m] << colli[n]
			<< colli[o] << colli[p]
			<< colli[q] << colli[r]
			<< colli[s] << colli[t] << std::endl;
	return 0;
}
