#ifndef STDADDRESSVIEW_H_INC
#define STDADDRESSVIEW_H_INC

#include <QWidget>
#include "../view.h"

namespace Standard {

	class AddressView : public ::View
	{
		Q_OBJECT

		public:
			AddressView(QWidget *parent = NULL, Document *doc = NULL);

		protected:
			void refreshPixmap();
	};

}


#endif
