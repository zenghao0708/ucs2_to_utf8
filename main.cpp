#define BUFFER_LEN		1024*1024*20
#define ARRAY_LEN(X)	(sizeof(X)/sizeof(X[0]))

unsigned char buffer_in[BUFFER_LEN]={0};
unsigned char buffer_out[BUFFER_LEN*3]={0};

bool unicode_convert_to_utf8(char * unicode_file,char * utf8_file)
{
	FILE * in,*out;
	unsigned char BOM[2]={0};
	unsigned char data[2]={0};
	unsigned short unicode_data=0;
	bool is_bigendian=false;

	in = fopen(unicode_file,"rb");
	if(NULL == in)
	{
		printf("cannot open files:%s\n",unicode_file);
		return false;
	}
	out = fopen(utf8_file,"wb");
	if(NULL == out)
	{
		printf("cannot open files:%s\n",utf8_file);
		return false;
	}

	fread(BOM,sizeof(BOM),1,in);

	if( BOM[0] !=0xFE && BOM[0] != 0xFF && BOM[1] !=0xFE && BOM[1] != 0xFF )
	{
		printf("the file: %s is not unicode file!",unicode_file);
		return false;
	}

	if(BOM[0] == 0xFE && BOM[1] == 0xFF)
		is_bigendian=true;

	int len=0,idx=0,out_idx=0;
	len=fread(buffer_in,sizeof(char),ARRAY_LEN(buffer_in),in);
	while(1)
	{
		while(idx <len)
		{
			memcpy(data,buffer_in+idx,ARRAY_LEN(data));
			idx +=2;
			if(!is_bigendian)
			{
				unicode_data = (data[1]<<8 & 0xFF00) |data[0];
			}else{
				unicode_data = (data[0]<<8 & 0xFF00) |data[1];
			}
		
			if( unicode_data <0x80 )
			{
				buffer_out[out_idx++]=(unsigned char)unicode_data;
			}else if( unicode_data <0x800)
			{
				unsigned char data_out[2] = {0};
				data_out[1] = 0x80 |(unicode_data & 0x3f);
				data_out[0] = 0xC0 | (unicode_data >> 6);
				memcpy(&buffer_out[out_idx],data_out,ARRAY_LEN(data_out));
				out_idx += ARRAY_LEN(data_out);
			}else{
				unsigned char data_out[3]={0};
				data_out[2] = 0x80 |(unicode_data & 0x3f);
				data_out[1] = 0x80 | ((unicode_data >> 6)& 0x3f);
				data_out[0] = 0xE0 | (unicode_data >> 12);
				memcpy(&buffer_out[out_idx],data_out,ARRAY_LEN(data_out));
				out_idx += ARRAY_LEN(data_out);
			}
		}
		fwrite(buffer_out,sizeof(char),out_idx,out);
		out_idx = 0;
		idx = 0;

		if(feof(in)) break;
		len=fread(buffer_in,sizeof(char),ARRAY_LEN(buffer_in),in);		
	}	

	fclose(in);
	fclose(out);

	return true;
}