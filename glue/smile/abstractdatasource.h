#ifndef DSL_ABSTRACTDATASOURCE_H
#define DSL_ABSTRACTDATASOURCE_H

// {{SMILE_PUBLIC_HEADER}}

#include <vector>

union DSL_dataElement;
struct DSL_variableInfo;

// a data source cannot be edited
// a data source can only be read once

class DSL_abstractDataSource
{
public:
	// get variable info
	virtual void GetVariablesInfo(std::vector<DSL_variableInfo> &here) = 0;
	virtual int NumVariables() const = 0; 
	// walk through the records 
	virtual bool NextRecord() = 0;
	// walk through the record elements
	virtual bool NextRecordElement(DSL_dataElement &here) = 0;
};

#endif
