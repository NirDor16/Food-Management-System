#pragma once
#include "Supermarket.h"

int	readDateFromBinaryFileCompressed(Date* pDate, FILE* fp);
int	readProductPriceFromBinaryFileCompressed(Product* pProduct, FILE* fp);
Product* readProductFromBinaryFileCompressed(FILE* fp);
int	readSuperMarketFromBinaryFileCompressed(SuperMarket* pSt, const char* fileName, const char* customersFileName);
int	saveSuperMarketToBinaryFileCompressed(const SuperMarket* pSt, const char* fileName, const char* customersFileName);
int	saveProductToBinaryFileCompressed(const Product* pProduct, FILE* fp);
int saveCountPriceToBinaryFileCompressed(const Product* pProduct, FILE* fp);
