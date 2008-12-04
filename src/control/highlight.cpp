
#include "highlight.h"
#include "document.h"

using namespace std;

Highlight::Highlight(Document *doc)
	: doc_(doc)
{
}

Highlight::~Highlight()
{
}

void Highlight::refresh()
{
}

bool Highlight::GetColor(vector<uchar> &buf, quint64 pos, uint size, HCIList &list)
{
	// sum(list[0].length, ..., list[N].length) = size となる
	return false;
}

