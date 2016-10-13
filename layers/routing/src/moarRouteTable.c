//
// Created by spiralis on 03.09.16.
//

#include "moarRouteTable.h"
#include <moarRouting.h>

const RouteAddr_T zeroAddress = {0};

int RouteTableInit(RouteDataTable_T * table, RouteTableSettings_T* settings) {
    if( NULL == table )
        return FUNC_RESULT_FAILED_ARGUMENT;

    table->Table = ( RouteDataRecord_T * )malloc( SzRouteDataRecord * settings->TableSize );

    if( NULL == table->Table )
        return FUNC_RESULT_FAILED_MEM_ALLOCATION;

    table->Count = 0;
    table->Capacity = settings->TableSize;
    table->LastTimeUpdated = 0;
    table->Settings = settings;
    return FUNC_RESULT_SUCCESS;
}

RouteDataTable_T * RouteTableCreate( RouteTableSettings_T* settings) {
    RouteDataTable_T * table = ( RouteDataTable_T * )malloc( sizeof( RouteDataTable_T ) );
    RouteTableInit( table, settings );
    return table;
}

int RouteTableClear( RouteDataTable_T * table ) {
    if( NULL == table )
        return FUNC_RESULT_FAILED_ARGUMENT;

    free( table->Table );

    table->Count = 0;
    table->Capacity = 0;
    table->LastTimeUpdated = 0;
    return FUNC_RESULT_SUCCESS;
}

int RouteTableDestroy( RouteDataTable_T * table ) {
    if( RouteTableClear( table ) ) {
        free( table );
        return FUNC_RESULT_SUCCESS;
    }
    return FUNC_RESULT_FAILED;
}

int RouteTableRenew( RouteDataTable_T * table, moarTime_T tick ) {
    moarTimeInterval_T	    renewRate;
    RouteTableSize_T        index,
                            count;
    RouteDataRecord_T	    *read,
                            *write;

    if( NULL == table || NULL == table->Settings )
        return FUNC_RESULT_FAILED_ARGUMENT;

    read = write = table->Table;
    count = index = 0;

    while( index < table->Count ) {
        if( read->P < table->Settings->RouteQualityThreshold )
            count++;

        else {
            *write = *read;
            renewRate = ( routeAddrEqual(write->Relay,zeroAddress) ) ? table->Settings->FinderMarkerRenewRate : table->Settings->RouteRenewRate;
            if( 0 == renewRate )
                write->P >>= 1;
            else if( write->P > renewRate )
                write->P -= renewRate;
            else
                write->P = 0;
            write++;
        }
        read++;
        index++;
    }

    table->Count -= count;
    table->LastTimeUpdated = tick;
    return ( count > 0 );
}

int RouteTableAdd( RouteDataTable_T * table, RouteAddr_T relay, RouteAddr_T dest ) {
    if( NULL == table || table->Count == table->Capacity )
        return FUNC_RESULT_FAILED_ARGUMENT;

    table->Table[ table->Count ].Dest = dest;
    table->Table[ table->Count ].Relay = relay;
    table->Table[ table->Count ].P = ( routeAddrEqual(relay,zeroAddress) ) ? table->Settings->FinderMarkerDefaultMetric : table->Settings->RouteDefaultMetric;
    table->Count++;
    //table->LastTimeUpdated = tick; //add this line! TODO
    return FUNC_RESULT_SUCCESS;
}

int RouteTableDelAll( RouteDataTable_T * table, RouteAddr_T relay, RouteAddr_T dest ) {
    RouteTableSize_T	index,
            count;
    RouteDataRecord_T	* read,
            * write;

    if( NULL == table )
        return FUNC_RESULT_FAILED_ARGUMENT;

    read = write = table->Table;
    count = index = 0;

    while( index < table->Count ) {
        if( routeAddrEqual(read->Relay, relay) && routeAddrEqual(read->Dest, dest) )
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

void RouteTableUpdate( RouteDataRecord_T * row ) {
    const RouteChance_T	half = ( ~( RouteChance_T )0 ) >> 1; // this way we find half of maximum value for P assuming it is not negative
    row->P = ( row->P >> 1 ) + half;
    //table->LastTimeUpdated = tick; //add this line! TODO
}

RouteDataRecord_T * RouteTableGetDest( RouteDataTable_T * table, RouteAddr_T relay ) {
    RouteTableSize_T	index;
    RouteDataRecord_T	* row,
            * ret;
    RouteChance_T		maxP = 0;

    for( ret = NULL, row = table->Table, index = 0; index < table->Count; row++, index++ )
        if( routeAddrEqual(relay, row->Relay) && maxP < row->P ) {
            ret = row;
            maxP = row->P;
        }

    return ret;
}

RouteDataRecord_T * RouteTableGetRelayFirst( RouteDataTable_T * table, RouteAddr_T dest ) {
    RouteTableSize_T	index = 0;

    if( NULL != table )
        while( index < table->Count )
            if( !routeAddrEqual(table->Table[ index ].Dest, dest) )
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
        if( routeAddrEqual(row->Dest, dest))
            return row;

    return NULL;
}

RouteDataRecord_T * RouteTableGetRelayBest( RouteDataTable_T * table, RouteAddr_T dest ) {
    RouteDataRecord_T	* row = RouteTableGetRelayFirst( table, dest ),
            * ret = row;

    while( NULL != row ) {
        if( ret->P < row->P )
            ret = row;

        row = RouteTableGetRelayNext( table, row );
    }

    return ret;
}

RouteDataRecord_T * RouteTableGetRecord( RouteDataTable_T * table, RouteAddr_T relay, RouteAddr_T dest ) {
    RouteTableSize_T	index;
    RouteDataRecord_T	* row;

    for( row = table->Table, index = 0; index < table->Count; row++, index++ )
        if( routeAddrEqual(dest, row->Dest) && routeAddrEqual(relay, row->Relay))
            return row;

    return NULL;
}

int Bump( RouteDataTable_T * table, RouteAddr_T relay, RouteChance_T newP ) {
    RouteTableSize_T	index, count;
    RouteDataRecord_T	* row;

    for( row = table->Table, count = 0, index = 0; index < table->Count; row++, index++ )
        if( routeAddrEqual(relay, row->Relay)) {
            row->P = newP;
            count++;
        }

    return ( count > 0 );
}

RouteDataRecord_T * RouteTableRowFirst( RouteDataTable_T * table ) {
    return ( NULL == table || 0 == table->Count ) ? NULL : table->Table;
}

RouteDataRecord_T * RouteTableRowNext( RouteDataTable_T * table, RouteDataRecord_T * prevRow ) {
    if( NULL == table || 0 == table->Count || NULL == prevRow )
        return NULL;
    else
        return ( ++prevRow >= ( table->Table + table->Count ) ) ? NULL : prevRow;
}

RouteDataRecord_T * RouteTableRowIndexed( RouteDataTable_T * table, RouteTableSize_T index ) {
    return ( NULL == table || index >= table->Count ) ? NULL : ( table->Table + index );
}

RouteTableSize_T RouteTableCount( RouteDataTable_T * table ) {
    return table->Count;
}
