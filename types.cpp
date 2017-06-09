#include "types.h"


void ItemStyle::debug(int verbosLevel) const
{
	if (verbosLevel < 0) {
		INFO << QString("mRange: %1 (%2)").arg(mRange.isValid()).arg(mRange.indexes().length())
			  << QString(" mSelection: %1").arg(mSelection.indexes().length())
			  << QString(" mIndexList: %1").arg(mIndexList.length())
			  << QString(" penColor: %1").arg(mPen.color().isValid());
		return;
	}
}


