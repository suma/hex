#ifndef STDHEXVIEW_H_INC
#define STDHEXVIEW_H_INC

#include "../view.h"

namespace Standard {


	class HexView : public ::View
	{
		Q_OBJECT

		public:
			HexView(QWidget *parent = NULL, Document *doc = NULL);

		protected:
			void refreshPixmap();
	};

}

#endif
