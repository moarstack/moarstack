//
// Created by spiralis on 03.09.16.
//

#ifndef MOARSTACK_MOARROUTETABLE_H
#define MOARSTACK_MOARROUTETABLE_H

#include <stdlib.h>
#include <stdint.h>
#include <moarCommons.h>
#include <moarTime.h>
#include <funcResults.h>
#include <moarRouting.h>

#define QualityThreshold	10
#define SzRouteDataRecord	sizeof(RouteDataRecord_T)

typedef uint16_t RouteTableSize_T;
typedef char	 RouteChance_T;

typedef struct{
	uint8_t TableSize;

	uint8_t FinderMarkerRenewRate;  //0 exp, 1..255 linear
	uint8_t FinderMarkerDefaultMetric;
	uint8_t RouteRenewRate; //0 exp, 1..255 linear
	uint8_t RouteDefaultMetric;
	uint8_t RouteQualityThreshold; // send finder if route less or equal
} RouteTableSettings_T;


typedef struct {
    RouteAddr_T		Relay,
            		Dest;
    RouteChance_T	P;
} RouteDataRecord_T;

typedef struct {
    RouteDataRecord_T		*Table;	// table itself
    RouteTableSize_T		Capacity,	// limit of records count in table
			            	Count;	// count of records in table
	moarTime_T				LastTimeUpdated;
	RouteTableSettings_T	*Settings;
} RouteDataTable_T;

int RouteTableInit(RouteDataTable_T * table, RouteTableSettings_T* settings);
RouteDataTable_T * RouteTableCreate( RouteTableSettings_T* settings);

int RouteTableClear(RouteDataTable_T * table );

int RouteTableDestroy(RouteDataTable_T * table );

int RouteTableRenew(RouteDataTable_T * table, moarTime_T tick );

int RouteTableAdd(RouteDataTable_T * table, RouteAddr_T relay, RouteAddr_T dest );

int RouteTableDelAll(RouteDataTable_T * table, RouteAddr_T relay, RouteAddr_T dest ); // add RouteTableDelOne() TODO
void RouteTableUpdate( RouteDataRecord_T * row );
RouteDataRecord_T * RouteTableGetDest( RouteDataTable_T * table, RouteAddr_T relay );
RouteDataRecord_T * RouteTableGetRelayFirst( RouteDataTable_T * table, RouteAddr_T dest );
RouteDataRecord_T * RouteTableGetRelayNext( RouteDataTable_T * table, RouteDataRecord_T * row );
RouteDataRecord_T * RouteTableGetRelayBest( RouteDataTable_T * table, RouteAddr_T dest );
RouteDataRecord_T * RouteTableGetRecord( RouteDataTable_T * table, RouteAddr_T relay, RouteAddr_T dest );

int Bump(RouteDataTable_T * table, RouteAddr_T relay, RouteChance_T newP );
RouteDataRecord_T * RouteTableRowFirst( RouteDataTable_T * table );
RouteDataRecord_T * RouteTableRowNext( RouteDataTable_T * table, RouteDataRecord_T * prevRow );
RouteDataRecord_T * RouteTableRowIndexed( RouteDataTable_T * table, RouteTableSize_T index );



#endif //MOARSTACK_MOARROUTETABLE_H
