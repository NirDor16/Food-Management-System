#include <stdio.h>
#include "string.h"
#include <stdlib.h>
#include <string.h>
#include "binaryComp.h"
#include "SuperFile.h"
#include "myMacros.h"

typedef unsigned char BYTE;


int readDateFromBinaryFileCompressed(Date* pDate, FILE* fp)
{
	BYTE data[2] = { 0 };

	if (fread(data, sizeof(BYTE), 2, fp) != 2)
		return 0;

	pDate->day = (data[0] >> 3) & 0x1F;

	pDate->month = ((data[0] & 0x07) << 1) | ((data[1] >> 7) & 0x01);

	pDate->year = ((data[1] >> 4) & 0x07) + 2024;

	return 1;
}


int	readProductPriceFromBinaryFileCompressed(Product* pProduct, FILE* fp)
{
	BYTE data[3] = { 0 };

	if (fread(data, sizeof(BYTE), 3, fp) != 3)
		return 0;
	pProduct->count = data[0] & 0xff;
	int cents = (data[1] >> 1) & 0x7f;
	pProduct->price = (float)((data[1] & 0x1) | (data[2] & 0xff));
	pProduct->price = ((pProduct->price * 100) + cents) / 100;
	return 1;
}

Product* readProductFromBinaryFileCompressed(FILE* fp)
{
	Product* pProduct = (Product*)malloc(sizeof(Product));
	if (!pProduct)
		return NULL;

	BYTE data[4] = { 0 };

	if (fread(data, sizeof(BYTE), 4, fp) != 4)
		return NULL;


	pProduct->type = (data[2] >> 2) & 0x3;
	if (pProduct->type < 0 || pProduct->type >= eNofProductType) return 0;
	const char* typePrefix = getProductTypePrefix(pProduct->type );

	pProduct->barcode[0] = typePrefix[0];
	pProduct->barcode[1] = typePrefix[1];
	

	int j = 0, i;
	for (i = 2; i < 6; i += 2)
	{
		pProduct->barcode[i] = ((data[j] >> 4) & 0xf) + '0'; // Convert the number to a character
		pProduct->barcode[i + 1] = (data[j] & 0xf) + '0';
		j++;
	}
	pProduct->barcode[i] = ((data[2] >> 4) & 0xf) + '0';
	pProduct->barcode[7] = '\0';

	int len_name = ((data[2] & 0x3) << 2) | ((data[3] >> 6) & 0x3);

	char* temp = (char*)calloc(len_name + 1, sizeof(char));
	if (!temp)
		return NULL;

	if (fread(temp, sizeof(char), len_name, fp) != len_name)
	{
		free(pProduct);
		return NULL;
	}
	strcpy(pProduct->name, temp);
	if (!pProduct->name)
	{
		free(pProduct);
		return NULL;
	}
	if (!readProductPriceFromBinaryFileCompressed(pProduct, fp))
	{
		free(pProduct);
		return NULL;
	}
	if (!readDateFromBinaryFileCompressed(&pProduct->expiryDate, fp))
	{
		free(pProduct);
		return NULL;
	}
	
	return pProduct;
}

int	readSuperMarketFromBinaryFileCompressed(SuperMarket* pSt, const char* fileName, const char* customersFileName)
{
	FILE* fp = fopen(fileName, "rb");
	CHECK_MSG_RETURN_0(fp);
	
	BYTE data[2] = { 0 };

	if (fread(data, sizeof(BYTE), 2, fp) != 2)
	{
		fclose(fp);
		return 0;
	}

	pSt->productCount = ((data[0] << 2) & 0xff) | ((data[1] >> 6) & 0x3);
	int len_name = data[1] & 0x3f;
	pSt->name = (char*)calloc(len_name + 1, sizeof(char));
	if (!pSt->name)
	{
		fclose(fp);
		return 0;
	}
	if (fread(pSt->name, sizeof(char), len_name, fp) != len_name)
	{
		pSt->name;
		fclose(fp);
		return 0;
	}

	pSt->productArr = (Product**)malloc(sizeof(Product) * pSt->productCount);
	if (!pSt->productArr)
	{
		pSt->name;
		fclose(fp);
		return 0;
	}

	for (int i = 0; i < pSt->productCount; i++)
	{
		pSt->productArr[i] = readProductFromBinaryFileCompressed(fp);
		if (!pSt->productArr[i])
		{
			free(pSt->name);
			for (int j = 0; j <= i; j++)
				free(pSt->productArr[j]);
			free(pSt->productArr);
			fclose(fp);
			return 0;
		}
	}
	pSt->customerArr = loadCustomersFromTextFile(customersFileName, &pSt->customerCount);
	fclose(fp);
	return 1;
}

int	saveSuperMarketToBinaryFileCompressed(const SuperMarket* pSt, const char* fileName, const char* customersFileName)
{
	FILE* fp = fopen(fileName,"wb");
	CHECK_MSG_RETURN_0(fp);

	int len = (int)strlen(pSt->name);
	BYTE data[2] = { 0 };
	
	data[0] = pSt->productCount >> 2;
	data[1] = (pSt->productCount & 0x3)<<6;
	data[1] = data[1] | len;
	
	if (fwrite(data, sizeof(BYTE), 2, fp) != 2)
	{
		fclose(fp);
		return 0;
	}
	if(fwrite(pSt->name,sizeof(char),len,fp)!=len)
	{
		fclose(fp);
		return 0;
	}
	for (int i = 0; i < pSt->productCount; i++)
	{
		if (!saveProductToBinaryFileCompressed(pSt->productArr[i], fp))
		{
			fclose(fp);
			return 0;
		}
	}
	saveCustomersToTextFile(pSt->customerArr, pSt->customerCount, customersFileName);
	fclose(fp);
	return 1;
}


int saveDateToBinaryFileCompressed(const Date* pDate, FILE* fp)
{
	BYTE data[2] = { 0 };

	data[0] = (pDate->day & 0x1F) << 3; 
	
	data[0] |= (pDate->month & 0x0F) >> 1;

	
	data[1] = (pDate->month & 0x01) << 7; 
	data[1] |= (pDate->year & 0x07) << 4; 

	if (fwrite(data, sizeof(BYTE), 2, fp) != 2)
		return 0;

	return 1;
}


int saveCountPriceToBinaryFileCompressed(const Product* pProduct, FILE* fp)
{
	BYTE data[3] = { 0 };
	data[0] = pProduct->count;
	int cents, priceWhole = (int)(pProduct->price * 100);
	cents = priceWhole % 100;
	priceWhole /= 100;
	data[1] = (cents << 1) | (priceWhole >> 8);
	data[2] = priceWhole & 0xFF;
	if (fwrite(data, sizeof(BYTE), 3, fp) != 3)
		return 0;
	return 1;
}


int saveProductToBinaryFileCompressed(const Product* pProduct, FILE* fp)
{
	BYTE data[4] = { 0 };
	int number , j = 0;
	for (int i = 2; i < BARCODE_LENGTH - 1; i+=2)
	{
		number = pProduct->barcode[i] - '0';
		data[j] = number << 4;
		number = pProduct->barcode[i + 1] - '0';
		data[j] = data[j] | number;
		j++;
	}
	number = pProduct->barcode[7] - '0';
	data[2] = number << 4;
	data[2] |= (pProduct->type << 2);
	int len = (int)strlen(pProduct->name);
	data[2] = data[2] | len >> 2;
	data[3] = len<<6;
	if (fwrite(data, sizeof(BYTE), 4, fp) != 4)
		return 0;
	if (fwrite(pProduct->name, sizeof(char), len, fp) != len)
		return 0;
	if (!saveCountPriceToBinaryFileCompressed(pProduct, fp))
		return 0;
	if (!saveDateToBinaryFileCompressed(&pProduct->expiryDate, fp))
		return 0;
	return 1;
}



