#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>

struct processingData
{
    int haystackBufferLength;
    char *buffer;
    char **lineBuffer;
    int uuidLength;
    int bufferSize;
    int *fileHandle;
    int threadID;
};

struct BackBuffer
{

    char *p_buff;
    int *fileHandle;
    int haystackBufferLength;
    int uuidLength;
    int used;
    int ready;
};

struct HashNode
{
    uint hash;
    char * value;
    struct HashNode * overflow;
};

struct HashMap 
{
    struct HashNode ** data;
    int size;
};

uint hashFunct(char * word)
{
uint hashAddress = 5381;
for (int counter = 0; word[counter]!='\n'&&word[counter] != '\0'; counter++){
    hashAddress = ((hashAddress << 5) + hashAddress) + word[counter];
}
return hashAddress;
}

struct HashMap* createHashMap(struct HashMap* map,size_t size)
{
    map = malloc(sizeof(struct HashMap));
    map->data = malloc((size*sizeof(struct HashNode))*2);
    map->size = size*2;
    return map;
}

void putInMap(struct HashMap* map,char * data)
{
    uint hash = hashFunct(data);
    if(!map->data[hash%map->size])
    {
        map->data[hash%map->size] = malloc(sizeof(struct HashNode));
        map->data[hash%map->size]->hash=hash;
        map->data[hash%map->size]->value = data;
        map->data[hash%map->size]->overflow = NULL;
    }
    else 
    {
       struct HashNode * node = map->data[hash%map->size];
             int nullcheck = 0;
        while(nullcheck == 0)
        {
            
                if(node->overflow)
                {
                node=node->overflow;
                }
                else
                {
                    nullcheck = 1;
                }
            
            
        }
       
        node->overflow = malloc(sizeof(struct HashNode));
        node->overflow ->hash=hash;
        node->overflow ->value = data;
        node->overflow->overflow = NULL;    
        

    }
    

}

int inMap(char * data,struct HashMap * map)
{
uint hash = hashFunct(data);
    if(map->data[hash%map->size] != NULL)
    {
       struct HashNode* node =  map->data[hash%map->size];
        if(map->data[hash%map->size]->hash == hash&&strcmp(data,node->value)==0)
        {
        return 0;
        }

        while(node->overflow)
        {
            node = node->overflow;
            if(node->hash == hash&&strcmp(data,node->value)==0)
            {
                return 0;
            }

        }
    }
    return 1;
}

int getMapSize(struct HashMap * map)
{
    int count = 0;
    for(int i = 0;i<map->size;i++)
    {
        struct HashNode* node = map->data[i];
    if(node)
    {
         while(node->overflow)
        {
            node = node->overflow;
            count++;

        } 
        count++;
    }
    
    }
    return count;

}



pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int totalProcessed = 0;
int totalRead = 0;
int totalSize;
size_t haySize = 0;
size_t chunks = 0;
int *matched;
struct HashMap* map;



void *processBuffer(void *processingDat)
{
    struct processingData *data = (struct processingData *)processingDat;
    int matchedBuffer[data->haystackBufferLength];
    pthread_t backBufferThread;
    int x = 0;
    char * line = malloc(data->uuidLength * sizeof(char));
    while (chunks < haySize)
    {

        size_t readSize = 0;
        /* memset(matchedBuffer, 0x00, sizeof(int) * data->haystackBufferLength); */

        
            readSize = read(*data->fileHandle, data->buffer, data->haystackBufferLength * data->uuidLength * sizeof(char));
            pthread_mutex_lock(&mutex);
            chunks += readSize;
            pthread_mutex_unlock(&mutex);
    

        int matches = 0;

  
            
            for (x = 0; x < data->haystackBufferLength; x++)
            {

                strncpy(line,data->buffer + (x * data->uuidLength),data->uuidLength);
            
                if ( inMap(line,map)==0)  //memcmp(data->buffer + (x * data->uuidLength), data->lineBuffer[y], data->uuidLength) == 0 )
                {
                    if(totalProcessed>=totalSize)
                    {
                        return NULL;
                    }

                    printf(line);

                    matches++;
                    
                    pthread_mutex_lock(&mutex);
                    totalProcessed++;
                
                    pthread_mutex_unlock(&mutex);
                   
            
                }
            
              
            }
        
  }  
}

int main(int argc, char *argv[])
{
    pthread_t *threadArray;
    struct stat charStat;
    errno = 0;
    FILE *needleFile;
    int hayFile;
    int needles = 0;
    char cwd[1024];
    int uuidLength = 38;
    int haystackBufferLength = 10000;

    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        //printf(cwd);
        //printf("\n");
    }

    for (int i = 0; i < argc; i++)
    {

        if (strcmp("--needles", argv[i]) == 0)
        {
            needleFile = fopen(argv[i + 1], "r");
            if (needleFile == NULL)
            {
                printf("file load failed for: ");
                printf(argv[i + 1]);
                //printf(errno);
                printf("\n");
            }
        }
        if (strcmp("--haystack", argv[i]) == 0)
        {
            stat(argv[i + 1], &charStat);
            haySize = charStat.st_size;
            hayFile = open(argv[i + 1], O_RDONLY);

            if (hayFile == -1)
            {
                printf("file load failed for: ");
                printf(argv[i + 1]);
                printf("\n");
            }
        }
    }

    int bufferSize = 32;
    
    char **lineBuffer = malloc(bufferSize * sizeof(*lineBuffer));
    char *line;
    line = malloc(uuidLength * sizeof(char));
    int i = 0;

    while (fgets(line, uuidLength, needleFile))
    {
        if (i < bufferSize)
        {

            lineBuffer[i] = malloc(256);

            strcpy(lineBuffer[i], line);

            i++;
        }
        if (i >= bufferSize)
        {
            char **tmp = realloc(lineBuffer, bufferSize * 2 * sizeof(*lineBuffer));
            if (tmp != NULL)
            {
                bufferSize *= 2;
                lineBuffer = tmp;
            }
        }

        // printf("%d",i);
    }
   

    bufferSize = i;
    int initalThreads = 8;

    map = createHashMap(map,bufferSize);
    for(i = 0;i<bufferSize;i++)
    {
        putInMap(map,lineBuffer[i]);
    }

    int mapSize = getMapSize(map);
        //char *buffer = malloc(haystackBufferLength * uuidLength * sizeof(char));
    int ch;

    int matches = 0;
    int readSize = 0;
    matched = malloc(sizeof(int)*bufferSize);
    uuidLength = strlen(lineBuffer[0]);
    threadArray = malloc(sizeof(pthread_t) * initalThreads);
    char threadBuffers[initalThreads][haystackBufferLength * uuidLength * sizeof(char)];
    struct processingData *buffData;
    int threads = 0;
    totalSize = bufferSize;
    if (totalProcessed >= bufferSize)
    {
        return 0;
    }

    // printf("chunks:");
    // printf("%zu",chunk);
    // printf(" ");
    // printf("%zu",haySize);
    // printf("\n");

    if (bufferSize > 100)
    {

        while (threads < initalThreads)
        {

            buffData = malloc(sizeof(struct processingData));

            buffData->buffer = threadBuffers[threads];
            buffData->bufferSize = bufferSize;
            buffData->uuidLength = uuidLength;
            buffData->lineBuffer = lineBuffer;
            buffData->haystackBufferLength = haystackBufferLength;
            buffData->fileHandle = &hayFile;
            buffData->threadID = threads;
            //threadArray[threads];
            pthread_create(&threadArray[threads], NULL, processBuffer, (void *)buffData);
            threads++;
        }
    }
    else
    {
        buffData = malloc(sizeof(struct processingData));

        buffData->buffer = threadBuffers[threads];
        buffData->bufferSize = bufferSize;
        buffData->uuidLength = uuidLength;
        buffData->lineBuffer = lineBuffer;
        buffData->haystackBufferLength = haystackBufferLength;
        buffData->fileHandle = &hayFile;
        buffData->threadID = 0;
        processBuffer((void *)buffData);
    }

    for (i = 0; i <= threads; i++)
        pthread_join(threadArray[i], NULL);

      //printf("%d",totalProcessed);
    // printf("%d",totalRead);
    return 0;
}