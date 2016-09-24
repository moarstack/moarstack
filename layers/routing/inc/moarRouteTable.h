//
// Created by spiralis on 03.09.16.
//

#ifndef MOARSTACK_MOARROUTETABLE_H
#define MOARSTACK_MOARROUTETABLE_H

#include <stdlib.h>
#include <stdint.h>
#include <moarCommons.h>
#include <moarTime.h>
#define QualityThreshold	10
#define SzRouteDataRecord	sizeof(RouteDataRecord_T)

typedef uint32_t RouteAddr_T;
typedef uint16_t RouteTableSize_T;
typedef char	 RouteChance_T;


typedef struct {
    RouteAddr_T		Relay,
            Dest;
    RouteChance_T	P;
} RouteDataRecord_T;

typedef struct {
    RouteDataRecord_T	* Table;	// table itself
    RouteTableSize_T	Capacity,	// limit of records count in table
            Count;	// count of records in table
    	moarTime_T	LastTimeUpdated;
} RouteDataTable_T;

bool RouteTableInit( RouteDataTable_T * table, RouteTableSize_T tableSize );
RouteDataTable_T * RouteTableCreate( RouteTableSize_T tableSize );
bool RouteTableClear( RouteDataTable_T * table );
bool RouteTableDestroy( RouteDataTable_T * table );
bool RouteTableRenew( RouteDataTable_T * table, moarTime_T tick );
bool RouteTableAdd( RouteDataTable_T * table, RouteAddr_T relay, RouteAddr_T dest );
bool RouteTableDelAll( RouteDataTable_T * table, RouteAddr_T relay, RouteAddr_T dest ); // add RouteTableDelOne() TODO
void RouteTableUpdate( RouteDataRecord_T * row );
RouteDataRecord_T * RouteTableGetDest( RouteDataTable_T * table, RouteAddr_T relay );
RouteDataRecord_T * RouteTableGetRelayFirst( RouteDataTable_T * table, RouteAddr_T dest );
RouteDataRecord_T * RouteTableGetRelayNext( RouteDataTable_T * table, RouteDataRecord_T * row );
RouteDataRecord_T * RouteTableGetRelayBest( RouteDataTable_T * table, RouteAddr_T dest );
RouteDataRecord_T * RouteTableGetRecord( RouteDataTable_T * table, RouteAddr_T relay, RouteAddr_T dest );
bool Bump( RouteDataTable_T * table, RouteAddr_T relay, RouteChance_T newP );
RouteDataRecord_T * RouteTableRowFirst( RouteDataTable_T * table );
RouteDataRecord_T * RouteTableRowNext( RouteDataTable_T * table, RouteDataRecord_T * prevRow );
RouteDataRecord_T * RouteTableRowIndexed( RouteDataTable_T * table, RouteTableSize_T index );



#endif //MOARSTACK_MOARROUTETABLE_H
