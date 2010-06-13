
#include <stdio.h>
#include "sndfile.h"
#include "autotalent.h"

#define BUF_SIZE 2048


void
Write16BitsLowHigh(FILE *fp, int i)
{
  putc(i & 0xff, fp);
  putc((i >> 8) & 0xff, fp);
}


void
Write32BitsLowHigh(FILE *fp, int i)
{
  Write16BitsLowHigh(fp, (int) (i & 0xffffL));
  Write16BitsLowHigh(fp, (int) ((i >> 16) & 0xffffL));
}


void
WriteWaveHeader(FILE * const fp, const int pcmbytes, const int freq, const int channels, const int bits)
{
  int bytes = (bits + 7) / 8;

  /* quick and dirty, but documented */
  fwrite("RIFF", 1, 4, fp); /* label */
  Write32BitsLowHigh(fp, pcmbytes + 44 - 8); /* length in bytes without header */
  fwrite("WAVEfmt ", 2, 4, fp); /* 2 labels */
  Write32BitsLowHigh(fp, 2 + 2 + 4 + 4 + 2 + 2); /* length of PCM format declaration area */
  Write16BitsLowHigh(fp, 1); /* is PCM? */
  Write16BitsLowHigh(fp, channels); /* number of channels */
  Write32BitsLowHigh(fp, freq); /* sample frequency in [Hz] */
  Write32BitsLowHigh(fp, freq * channels * bytes); /* bytes per second */
  Write16BitsLowHigh(fp, channels * bytes); /* bytes per sample time */
  Write16BitsLowHigh(fp, bits); /* bits per sample */
  fwrite("data", 1, 4, fp); /* label */
  Write32BitsLowHigh(fp, pcmbytes); /* length in bytes of raw PCM data */
}

int
main(int argc, char **argv)
{
  SNDFILE *ifp, *ofp;
  SF_INFO *if_info, *of_info;

  if (argc != 3)
  {
    /* display not so helpful message */
    printf("Usage: autotalent <in file> <out file>\n");
  }
  else
  {
    if (strcmp(argv[1], argv[2]) != 0)
    {
      /* open input files through libsndfile */
      if_info = (SF_INFO *)calloc(1, sizeof(SF_INFO));
      ifp = sf_open(argv[1], SFM_READ, if_info);

      if (ifp != NULL)
      {
        /* open output file */
        of_info = (SF_INFO *)calloc(1, sizeof(SF_INFO));
        of_info->samplerate = if_info->samplerate;
        of_info->channels = if_info->channels;
        of_info->format = if_info->format;
        ofp = sf_open(argv[2], SFM_WRITE, of_info);
      }
      else
      {
        printf("Unable to open input file!\n");
        exit(EXIT_FAILURE);
      }

      if (ofp != NULL)
      {
        /* set up autotalent */
        Autotalent * autotalent = getAutotalentInstance();
        setAutotalentKey(autotalent, 'c');
        setAutotalentParameters(autotalent, 440, 0, 0.2f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 5.0f, 0.0, 0.0f, 0, 0, 0, 0.0f, 0.5f);

        /* set up buffer we're running through autotalent */
        int total_samples = 0;
        sf_count_t samples_read = 0;
        float *buf = (float *)calloc(BUF_SIZE, sizeof(float));

        do {
          /* read samples */
          samples_read = sf_read_float(ifp, buf, BUF_SIZE);
          total_samples += samples_read;

          /* run buffer through autotalent */
          setAutotalentBuffers(autotalent, buf, buf);
          runAutotalent(autotalent, samples_read);

          /* write to output file */
          sf_write_float(ofp, buf, samples_read);
        } while (samples_read > 0);

        freeAutotalentInstance();
        free(buf);
        sf_close(ifp);
        sf_close(ofp);
      }
      else
      {
        printf("Unable to open output file!\n");
        exit(EXIT_FAILURE);
      }
    }
    else
    {
      /* we can't read and write to the same file, sorry */
      printf("attempting to write to the same file!\n");
      exit(EXIT_FAILURE);
    }
  }

  return 0;
}
