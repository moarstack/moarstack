//
// Created by spiralis on 03.09.16.
//

#include "moarRouteTable.h"

bool RouteTableInit(RouteDataTable_T *table, RouteTableSize_T tableSize) {
    if( NULL == table )
        return false;

    table->Table = ( RouteDataRecord_T * )malloc( SzRouteDataRecord * tableSize );

    if( NULL == table->Table )
        return false;

    table->Count = 0;
    table->Capacity = tableSize;
    table->LastTimeUpdated = 0;
    return true;

}

bool RouteTableClear(RouteDataTable_T *table) {
    if( NULL == table )
        return false;

    free( table->Table );

    table->Count = 0;
    table->Capacity = 0;
    table->LastTimeUpdated = 0;
    return true;

}

bool RouteTableDestroy(RouteDataTable_T *table) {
    if( RouteTableClear( table ) ) {
        free( table );
        return true;
    }
    return false;
}

bool RouteTableRenew(RouteDataTable_T *table, moarTime_T tick) {
    RouteTableSize_T	index,
            count;
    RouteDataRecord_T	* read,
            * write;
    uint32_t			interval;

    if( NULL == table )
        return false;

    read = write = table->Table;
    count = index = 0;
    interval = ( uint32_t )( tick - table->LastTimeUpdated );

    while( index < table->Count ) {
        if( read->P < QualityThreshold ){
            count++;
            read = NULL;
        }
        else {
            *write = *read;
            write->P = ( write->P >> 1 );// - ( RouteChance_T )( interval / ( uint32_t )( write->P ) );
            write++;
        }
        read++;
        index++;
    }

    table->Count -= count;
    table->LastTimeUpdated = tick;
    return ( count > 0 );
}

bool RouteTableAdd(RouteDataTable_T *table, RouteAddr_T relay, RouteAddr_T dest) {
    if( NULL == table || table->Count >= table->Capacity )
        return false;

    table->Table[ table->Count ].Dest = dest;
    table->Table[ table->Count ].Relay = relay;
    table->Table[ table->Count ].P = ( ~( RouteChance_T )0 ) >> 1;
    table->Count++;
    //table->LastTimeUpdated = tick; //add this line! TODO
    return true;
}

bool RouteTableDelAll(RouteDataTable_T *table, RouteAddr_T relay, RouteAddr_T dest) {
    RouteTableSize_T	index,
            count;
    RouteDataRecord_T	* read,
            * write;

    if( NULL == table )
        return false;

    read = write = table->Table;
    count = index = 0;

    while( index < table->Count ) {
        if( read->Relay == relay && read->Dest == dest )
            count++;

        else {
            *write = *read;
            write++;
        }
        read++;
        index++;
    }

    table->Count -= count;
    //table->LastTimeUpdated = tick; //add this line! TODO
    return ( count > 0 );
}

inline static void RouteTableUpdate(RouteDataRecord_T *row) {
    const RouteChance_T	half = ( ~( RouteChance_T )0 ) >> 1; // this way we find half of maximum value for P assuming it is not negative
    row->P = ( row->P >> 1 ) + half;
    //table->LastTimeUpdated = tick; //add this line! TODO

}

RouteDataRecord_T *RouteTableGetDest(RouteDataTable_T *table, RouteAddr_T relay) {
    	RouteTableSize_T	index;
	RouteDataRecord_T	* row,
						* ret;
	RouteChance_T		maxP = 0;

	for( ret = NULL, row = table->Table, index = 0; index < table->Count; row++, index++ )
		if( relay == row->Relay && maxP < row->P ) {
			ret = row;
			maxP = row->P;
		}

	return ret;
}

RouteDataRecord_T *RouteTableGetRelayFirst(RouteDataTable_T *table, RouteAddr_T dest) {
    RouteTableSize_T	index = 0;

    if( NULL != table )
        while( index < table->Count )
            if( table->Table[ index ].Dest != dest )
                index++;
            else
                return table->Table + index;

    return NULL;
}

RouteDataRecord_T * RouteTableGetRelayNext( RouteDataTable_T * table, RouteDataRecord_T * row ) {
    if( NULL == table || NULL == row )
        return NULL;

    RouteDataRecord_T	* end = table->Table + table->Count;
    RouteAddr_T			dest = row->Dest;

    while( ++row != end )
        if( row->Dest == dest )
            return row;

    return NULL;

}

RouteDataRecord_T *RouteTableGetRelayBest(RouteDataTable_T *table, RouteAddr_T dest) {
    RouteDataRecord_T	* row = RouteTableGetRelayFirst( table, dest ),
            * ret = row;

    while( NULL != row ) {
        if( ret->P < row->P )
            ret = row;

        row = RouteTableGetRelayNext( table, row );
    }

    return ret;
}

RouteDataRecord_T *RouteTableGetRecord(RouteDataTable_T *table, RouteAddr_T relay, RouteAddr_T dest) {
    RouteTableSize_T	index;
    RouteDataRecord_T	* row;

    for( row = table->Table, index = 0; index < table->Count; row++, index++ )
        if( dest == row->Dest && relay == row->Relay )
            return row;

    return NULL;
}

bool Bump(RouteDataTable_T *table, RouteAddr_T relay, RouteChance_T newP) {
    RouteTableSize_T	index, count;
    RouteDataRecord_T	* row;

    for( row = table->Table, count = 0, index = 0; index < table->Count; row++, index++ )
        if( relay == row->Relay ) {
            row->P = newP;
            count++;
        }

    return ( count > 0 );
}

inline static RouteDataRecord_T *RouteTableRowFirst(RouteDataTable_T *table) {
    return ( NULL == table || 0 == table->Count ) ? NULL : table->Table;
}

inline static RouteDataRecord_T *RouteTableRowNext(RouteDataTable_T *table, RouteDataRecord_T *prevRow) {
    if( NULL == table || 0 == table->Count || NULL == prevRow )
        return NULL;
    else
        return ( ++prevRow >= ( table->Table + table->Count ) ) ? NULL : prevRow;
}

inline static RouteDataRecord_T *RouteTableRowIndexed(RouteDataTable_T *table, RouteTableSize_T index) {
    return ( NULL == table || index >= table->Count ) ? NULL : ( table->Table + index );
}

inline static RouteDataTable_T *RouteTableCreate(RouteTableSize_T tableSize) {
    RouteDataTable_T * table = ( RouteDataTable_T * )malloc( sizeof( RouteDataTable_T ) );
    RouteTableInit( table, tableSize );
    return table;
}
