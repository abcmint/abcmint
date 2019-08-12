#include <cuda.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <sys/time.h>
#include <inttypes.h>

#define K PARK

#define NUM_THREADS (1 << (N-K))
#define BLOCK_DIM (NUM_THREADS > 128 ? 128 : NUM_THREADS)
#define GRID_DIM (NUM_THREADS/BLOCK_DIM)

//#define PRINT_SOL(X) printf("%lX\n", X)
#define PRINT_SOL(X)

//#define LOG(level, f_, ...) fprintf(stderr, (f_), ##__VA_ARGS__)
#define LOG(level, f_, ...)


#define COEF(I,J) ((((J)*((J)-1))>>1) + (I))
#if 0
/* for parsing challenge file */
const char* CHA_GF_LINE = "Galois Field";
const char* CHA_VAR_LINE = "Number of variables";
const char* CHA_EQ_LINE = "Number of polynomials";
const char* CHA_SEED_LINE = "Seed";
//const char* CHA_ORD_LINE = "Order";
const char* CHA_EQ_START = "*********";
const size_t MAX_PRE_LEN = 128;
#endif

template <class Type>
class cudaData
{
public:
	cudaData(size_t len_host, size_t len_dev=0);
	~cudaData();

	size_t size_host();
	size_t size_dev();
	void clear();
	void write(size_t off_src=0, size_t size=0, size_t off_des=0);		
	void read(size_t off_src=0, size_t size=0, size_t off_des=0);

	Type *host;
	Type *dev;

private:
	size_t sz_host;
	size_t sz_dev;
};


template <class Type>
cudaData<Type>::cudaData(size_t len_host, size_t len_dev)
{
	if(len_dev == 0) len_dev = len_host;

	sz_host = len_host * sizeof(Type);
	sz_dev  = len_dev * sizeof(Type);

	host = (Type*) malloc(sz_host);	

	cudaMalloc((void**) &dev, sz_dev);
}

template <class Type>
cudaData<Type>::~cudaData()
{       if(host)
	free(host);
        if(dev)
	cudaFree(dev);
}

template <class Type>
size_t cudaData<Type>::size_host()
{
	return sz_host;
}

template <class Type>
size_t cudaData<Type>::size_dev()
{
	return sz_dev;
}

template <class Type>
void cudaData<Type>::clear()
{
	memset(host, 0, sz_host);
}

template <class Type>
void cudaData<Type>::write(size_t off_src, size_t size, size_t off_des)
{
	if(size == 0) size = (sz_host <= sz_dev) ? sz_host : sz_dev;

	cudaMemcpy(&dev[off_src], &host[off_des], size, cudaMemcpyHostToDevice);
}

template <class Type>
void cudaData<Type>::read(size_t off_src, size_t size, size_t off_des)
{
	if(size == 0) size = (sz_host <= sz_dev) ? sz_host : sz_dev;

	cudaMemcpy(&host[off_src], &dev[off_des], size, cudaMemcpyDeviceToHost);
}

#if 0
/* testing if pre is a prefix of the string */
static inline bool
check_prefix(const char* pre, const char* str) {
      return !strncmp(pre, str, strnlen(pre, MAX_PRE_LEN));
}

/* parse the header of challenge file, return true is still in header.
    * return false otherwise.
     */
static bool
parse_cha_header(const char* str, uint32_t &N, uint32_t &M) {
  bool verbose = false;
  if(check_prefix(CHA_EQ_START, str)) {
    if(verbose) {
      printf("\t\treading equations...\n");
    }
    return false;
  }

  uint64_t var_num, eq_num, seed;

  if(check_prefix(CHA_VAR_LINE, str)) {
    if(1 != sscanf(str, "%*s %*s %*s %*s : %" PRIu64, &var_num)) {
      fprintf(stderr, "[!] cannot parse number of unknowns: %s\n", str);
      exit(-1);
    }

    N = var_num;

//    if (var_num != N)
//    {
//      fprintf(stderr, "Number of variables in input file does not fit compile options!\n");
//      fprintf(stderr, "%" PRIu64 " != %i\n", var_num, N);
//      exit(-1);
//    }

    if(verbose) {
      printf("\t\tnumber of variables: %" PRIu64 "\n", var_num);
    }

  } else if(check_prefix(CHA_EQ_LINE, str)) {
    if(1 != sscanf(str, "%*s %*s %*s %*s : %" PRIu64, &eq_num)) {
      fprintf(stderr, "[!] cannot parse number of equations: %s\n", str);
      exit(-1);
    }

    M = eq_num;

//    if (eq_num != M)
//    {
//      fprintf(stderr, "Number of equations in input file does not fit compile options!\n");
//      fprintf(stderr, "%" PRIu64 " != %i\n", eq_num, M);
//      exit(-1);
//    }

    if(verbose) {
      printf("\t\tnumber of equations: %" PRIu64 "\n", eq_num);
    }

  } else if(check_prefix(CHA_SEED_LINE, str)) {
    if(1 != sscanf(str, "%*s : %" PRIu64, &seed)) {
      fprintf(stderr, "[!] unable to seed: %s\n", str);
      exit(-1);
    }

    if(verbose) {
      printf("\t\tseed: %" PRIu64 "\n", seed);
    }

  } else if(check_prefix(CHA_GF_LINE, str)) {
    int prime = 0;
    if( (1 != sscanf(str, "%*s %*s : GF(%d)", &prime)) || prime != 2) {
      fprintf(stderr, "[!] unable to process GF(%d)\n", prime);
      exit(-1);
    }

    if(verbose) {
      printf("\t\tfield: GF(%d)\n", prime);
    }
  }

  return true;
}

/* parse the system of challenge file. Note this will destroy the string */
static void
parse_cha_eqs(char* str, const uint64_t eq_idx, uint32_t *orig_sys, uint32_t N) {
  char* ptr = NULL;

  uint64_t i = 0;
  ptr = strtok(str, " ;");
  while(NULL != ptr) {
    orig_sys[(N*(N-1)/2 + N + N + 1)*eq_idx +i] = atoi(ptr);
    i += 1;
    ptr = strtok(NULL, " ;\n");
  }
}

uint32_t *read_sys(uint32_t &N, uint32_t &M)
{
  FILE* fp = stdin;
  //FILE* fp = fopen( "data.in" , "r");

  // NOTE: expand the buffer if needed
  const size_t buf_size = 0x1 << 20; // 1MB per line
  char* buf = (char*)malloc(buf_size);
  uint64_t eq_idx = 0;

  while (NULL != fgets(buf, buf_size, fp)) {
    if (!parse_cha_header(buf, N, M))
      break;
  }

  if (feof(fp))
    return NULL;

  uint32_t* data = (uint32_t*)malloc((N*(N-1)/2 + 2*N + 1)*M*sizeof(uint32_t));

  for (int i = 0; i < M; i++) {
    if (NULL != fgets(buf, buf_size, fp)) {
      parse_cha_eqs(buf, eq_idx++, data, N);
    }
    else
    {
      free(buf);
      free(data);

      fprintf(stderr, "Error while reading input data!\n");
      exit(-1);
    }
  }

  if (feof(fp))
    fprintf(stderr, "end of file\n");

  //fclose(fp);
  free(buf);

  return data;
}

#endif

#ifndef HAVE_CNT
static const int MultiplyDeBruijnBitPosition[32] = 
{
    0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8, 
      31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
};

static uint32_t cnt0(uint32_t v)
{
  return MultiplyDeBruijnBitPosition[((uint32_t)((v & -v) * 0x077CB531U)) >> 27];
}
#define HAVE_CNT
#endif


uint32_t* check_thread(uint32_t *deg2, uint32_t *deg1, uint32_t thread, uint32_t N)
{
  uint32_t rounds;
  uint32_t x = 0;
  uint32_t y = 0;
  uint32_t z = 0;
  uint32_t tmp = 0;
  uint32_t count = 0;

  uint32_t diff[K];

  uint32_t* result = (uint32_t*) malloc( (1 << K) * sizeof(uint32_t) );

  diff[0] = deg1[0 * NUM_THREADS + thread];

  for (int i = 1; i < K; i++)
  {
    diff[i] = deg1[i * NUM_THREADS + thread] ^ deg2[COEF(i-1, i)];
  }

  uint32_t res = deg1[K * NUM_THREADS + thread];

  for( rounds = 1; rounds < (1 << K); rounds += 1)
  {
    tmp = (rounds & (rounds-1));
    y = rounds ^ tmp;
    x ^= y;
    z = tmp ^ (tmp & (tmp-1));

    uint32_t y_pos = cnt0(y);
    uint32_t z_pos = cnt0(z);

    if (z_pos > y_pos)
      diff[y_pos] ^= deg2[COEF(y_pos, z_pos)];

    res ^= diff[y_pos];
    if( res == 0 ) result[ count++ ] = x;
  }

  result[ count ] = 0;

  return result;
}


void deg0_coefs(uint32_t *deg2, uint32_t *deg1, uint32_t *result, uint32_t N)
{
	uint32_t rounds;
	uint32_t x = 0;
	uint32_t y = 0;
	uint32_t z = 0;
	uint32_t tmp = 0;

  uint32_t diff[N-K];

	diff[0] = deg1[0];

  for (int i = 1; i < (N-K); i++)
  {
    diff[i] = deg1[i] ^ deg2[COEF(i-1, i)];
  }

	uint32_t res = deg1[N-K];

	result[0] = res;

	for (rounds = 1; rounds < (1 << (N-K)); rounds += 1)
	{
		tmp = (rounds & (rounds-1));
		y = rounds ^ tmp;
		x ^= y;
		z = tmp ^ (tmp & (tmp-1));

    uint32_t y_pos = cnt0(y);
    uint32_t z_pos = cnt0(z);

    if (z_pos > y_pos)
      diff[y_pos] ^= deg2[COEF(y_pos, z_pos)];

    res ^= diff[y_pos];
    result[ x ] = res;
    tmp = (y_pos * (y_pos-1)) >> 1;
	}
}

void deg1_coefs(uint32_t *deg1, uint32_t *result, uint32_t N)
{
	uint32_t x = 0;
	uint32_t y = 0;
	uint32_t res = deg1[N-K];

	result[0] = res;

	for(uint32_t rounds = 1; rounds < (1 << (N-K)); rounds += 1)
	{
		y = rounds ^ (rounds & (rounds-1));
		x ^= y;

    res ^= deg1[cnt0(y)];
    result[ x ] = res;
	}
}

void partial_eval(uint32_t *sys, uint32_t *deg1, uint32_t N)
{
	uint32_t deg1_sys[(N-K)+1];
	uint32_t deg2_sys[COEF(N-K, N-K)+1];
	uint64_t pos = 0;

	// deg2 part
	for(uint32_t i = 0; i < K; i++)
	{
    for(uint32_t j = 0; j <= (N-K); j++)
    {
      deg1_sys[j] = sys[COEF(0, j + K) + i];
    }

		deg1_coefs(deg1_sys, &deg1[pos], N);
		pos += (1 << (N-K));
	}

	// deg1 part
  for(uint32_t j = 1; j <= (N-K); j++)
  {
    for(uint32_t i = 0; i <= j; i++)
    {
      deg2_sys[COEF(i, j)] = sys[COEF(i + K, j + K)];
    }
  }

  deg0_coefs(deg2_sys, deg2_sys + COEF(0, N-K), &deg1[pos], N);
}



uint32_t check_sol(uint32_t *sys, uint64_t sol, uint32_t N, uint32_t M)
{
	uint32_t i, j, pos = 0;
	uint32_t x[N], check = 0;
   
  for (uint32_t b = 0; b < M; b +=32)
  {
    uint32_t mask = (M-b) >= 32 ? 0xffffffff : ((1 << (M-b))-1);

    for (i = 0; i < N; i++)
      x[i] = ((sol >> i) & 1) ? mask : 0;

    // computing quadratic part
    for(j = 1; j < N; j++)
      for(i = 0; i < j; i++) 
        check ^= sys[pos++] & x[i] & x[j];

    // computing linear part
    for(i = 0; i < N; i++) 
      check ^= sys[pos++] & x[i];

    // constant part
    check ^= sys[pos++];
  }

	return check;
}



double get_ms_time(void) {
	struct timeval timev;

	gettimeofday(&timev, NULL);
	return (double) timev.tv_sec * 1000 + (double) timev.tv_usec / 1000;
}

extern uint32_t check_sol(uint32_t *sys, uint64_t sol, uint32_t N, uint32_t M);
extern void partial_eval(uint32_t *sys, uint32_t *deg1, uint32_t N);
extern uint32_t* check_thread(uint32_t *deg2, uint32_t *deg1, uint32_t thread, uint32_t N);

__device__ __constant__ uint32_t deg2_block[ K*(K-1)/2 ];

#include "kernel.inc"

uint32_t *pack_sys_data(uint32_t *data, uint32_t N, uint32_t M)
{
//  reduce input system - remove squares

  uint32_t num_blocks = ((M >> 5) + ((M & 31) == 0 ? 0 : 1));

  uint32_t *sys = (uint32_t*)malloc(sizeof(uint32_t)*(N*(N-1)/2 + N + 1)*num_blocks);
  uint32_t *sq0 = (uint32_t*)malloc(sizeof(uint32_t)*N*num_blocks);


  int sq_id = 0;

  int is = 0;
  int id = 0;
  
  for (int v0 = 0; v0 < N; v0++)
  {
    for (int v1 = 0; v1 <= v0; v1++)
    {
      for (uint32_t b = 0; b < M; b +=32)
      {
        uint32_t val = 0;

        for (int j = (((M - b) >= 32) ? b + 31 : (M-1)); j >= (int)b; j--)
          val = (val << 1) | data[(N*(N-1)/2 + N + N + 1) * j + is];

        if (v0 == v1)
          sq0[sq_id + N*(b >> 5)] = val;
        else
          sys[(N*(N-1)/2 + N + 1) * (b >> 5) + id] = val;
      }

      is += 1;

      if (v0 == v1)
        sq_id += 1;
      else
        id += 1;
    }
  }

  for (int v0 = 0; v0 < N; v0++)
  {
    for (uint32_t b = 0; b < M; b +=32)
    {
      uint32_t val = 0;

      for (int j = (((M - b) >= 32) ? b + 31 : (M-1)); j >= (int)b; j--)
        val = (val << 1) | data[(N*(N-1)/2 + N + N + 1) * j + is];

      sys[(N*(N-1)/2 + N + 1) * (b >> 5) + id] = val ^ sq0[v0 + N*(b >> 5)];
    }

    is += 1;
    id += 1;
  }

  {
    for (uint32_t b = 0; b < M; b +=32)
    {
      uint32_t val = 0;

      for (int j = (((M - b) >= 32) ? b + 31 : (M-1)); j >= (int)b; j--)
        val = (val << 1) | data[(N*(N-1)/2 + N + N + 1) * j + is];

      sys[(N*(N-1)/2 + N + 1) * (b >> 5) + id] = val;
    }
  }
//  if(data)
//  free(data);
  if(sq0)
  free(sq0);
  return sys;
}
static int cuda_device = 0;
static bool init = false;

void SetDevice(int device)
{
  cuda_device = device;
  init = false;
}

int  GetDeviceCount() {
    int deviceCount, result;
    cudaGetDeviceCount(&deviceCount);
    result = deviceCount;
    return result;
}


uint64_t GPUSearchSolution(uint32_t* coefficients, unsigned int number_of_variables, 
                           unsigned int number_of_equations)
{

  if (!init)
  {
  //  double initTime = 0;
  //  initTime -= get_ms_time();

    // set to designated device
    //int test;
    cudaSetDevice(cuda_device);
    //cudaGetDevice(&test);
    //assert(atoi(argv[1]) == test);

  //  initTime += get_ms_time();
  //  LOG(INFO, "init time = %f\n", initTime);

    init = true;
  }


//  double preTime = 0, memTime = 0, recvTime = 0, checkTime = 0, ctTime = 0;
//  float kernelTime = 0;
  uint32_t solCount = 0, ctCount = 0;

  uint64_t res = 0;

  // create events here
  cudaEvent_t start, stop;
  cudaEventCreate (&start);
  cudaEventCreate (&stop);


  uint32_t N = number_of_variables;
  uint32_t M = number_of_equations;


  if (N <= K)
  {
    fprintf(stderr, "N must be larger than K!\n");
    exit(-1);
  }

  uint32_t *sys = pack_sys_data(coefficients, N, M);

//  preTime -= get_ms_time(); // partial evaluation

  cudaData<uint32_t> deg1((K + 1) * NUM_THREADS);

  partial_eval(sys, deg1.host, N);

//  preTime += get_ms_time();

//  memTime -= get_ms_time(); // initializing GPU memory space

  // initialize constant memory space for the quadratic part
  cudaMemcpyToSymbol(deg2_block, sys, sizeof(uint32_t) * K*(K-1)/2);

  // initialize global memory space for the linear parts
  deg1.write();

  // initialize global memory space for the results of each threads
  cudaData<uint32_t> result(NUM_THREADS);

 // memTime += get_ms_time(); 

  // launch kernel function and measure the elapsed time
//  cudaEventRecord(start, 0);

  guess<<<GRID_DIM, BLOCK_DIM>>>(deg1.dev, result.dev, NUM_THREADS);

//  cudaEventRecord(stop, 0);
//  cudaEventSynchronize(stop);

//  cudaEventElapsedTime(&kernelTime, start, stop);

 // recvTime -= get_ms_time(); // copy the results of each thread to host

  result.read();

//  recvTime += get_ms_time();

//  checkTime -= get_ms_time(); // check if the results are available

  int32_t ans; 

  for(uint64_t i = 0; i < NUM_THREADS; i++)
  {
    ans = result.host[i];

    if(ans)
    {
      solCount++;

      if (ans & 0x80000000) // more than one solution 
      {
        uint32_t *sols;

        ctCount++;
     //   ctTime -= get_ms_time();
        sols = check_thread(sys, deg1.host, i, N);
     //   ctTime += get_ms_time();

        uint32_t j;

        for(j = 0; sols[j]; j++)
        {
          if (check_sol(sys, (i << K) | sols[j], N, M) == 1)
          {
         //   LOG(INFO, "thread %lX ---------> solution %X\n", i, sols[j]);
            PRINT_SOL((i << K) | sols[j]);

            res = (i << K) | sols[j];
            free(sols);
            goto end;
           
          }
        }

       // LOG(INFO, "thread %lX ---------> several solutions: %u\n", i, j);

        free(sols);
      }
      else // only one solution
      {
        if (check_sol(sys, (i << K) | ans, N, M) == 0)
        {
     //     LOG(INFO, "thread %lX ---------> one solution %X\n", i, ans);
          PRINT_SOL((i << K) | ans);

          res = (i << K) | ans;

          goto end;
        }
      }
    }

    if(deg1.host[K * NUM_THREADS + i] == 0)
    {
      if (check_sol(sys, (i << K) | 0, N, M) == 0)
      {
   //     LOG(INFO, "thread %lX ---------> one solution 0\n", i);
        PRINT_SOL(i << K);

        res = (i << K);

        goto end;
      }
    }
  }

end:

 // checkTime += get_ms_time();
#if 0
  // print the time for each step
  LOG(INFO, "partial ");
  LOG(INFO, "mem ");
  LOG(INFO, "kernel ");
  LOG(INFO, "recv ");
  LOG(INFO, "check #sol ");
  LOG(INFO, "(mult sol: t #ct)\n");
  LOG(INFO, "%.3f ", preTime);
  LOG(INFO, "%.3f ", memTime);
  LOG(INFO, "%.3f ", kernelTime);
  LOG(INFO, "%.3f ", recvTime);
  LOG(INFO, "%.3f ", checkTime);
  LOG(INFO, "%u ", solCount);
  LOG(INFO, "(%.3f  %u)\n", ctTime, ctCount);
#endif
  // release memory spaces
  if(sys)
  free(sys);

  return res;
}
