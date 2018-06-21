#include <stdio.h>
#include <stdlib.h>

void
HSVtoRGB(float h, float s, float v, float rgb[3]) {
  if(s == 0.0) {
    rgb[0] = v;
    rgb[1] = v;
    rgb[2] = v;
  } else {
    int i;
    float f, p, q, t;
    if(h == 360) h = 0;
    h /= 60;
    i = (int)h;
    f = h - i;
    p = v * (1 - s);
    q = v * (1 - (s * f));
    t = v * (1 - (s * (1 - f)));
    switch(i) {
    case 0: rgb[0] = v; rgb[1] = t; rgb[2] = p; break;
    case 1: rgb[0] = q; rgb[1] = v; rgb[2] = p; break;
    case 2: rgb[0] = p; rgb[1] = v; rgb[2] = t; break;
    case 3: rgb[0] = p; rgb[1] = q; rgb[2] = v; break;
    case 4: rgb[0] = t; rgb[1] = p; rgb[2] = v; break;
    case 5: rgb[0] = v; rgb[1] = p; rgb[2] = q; break;
    default: abort();
    }
  }
}

int
main(int argc, char *argv[]) {
  const char *filename;
  char out_filename[1024];
  FILE *file;
  int w, h, x, y, i, j;
  float z, *matrix, color[3];
  unsigned char *ppm;
  int ppm_w, ppm_h;

  int magic_id_c = 0xfe76;
  int version_c = 2;
  int magic_id;
  int version;
  int start[2];
  int end[2];
  int step[2];
  int param[2];
  int ndim;
  double *kc;
  int fitness_func;
  
  for(i=1; i<argc; i++) {
    filename = argv[i];    
    file=fopen(filename, "rb");
    if(!file) {
      fprintf(stderr, "Unable to open %s\n", filename);
      continue;
    }
    fread(&magic_id, sizeof(magic_id), 1, file);
    if(magic_id != magic_id_c) abort();
    fread(&version, sizeof(version), 1, file);
    if(version != version_c) abort();
    fread(&fitness_func, sizeof(fitness_func), 1, file);
    fread(param, sizeof(param), 1, file);
    fread(start, sizeof(start), 1, file);
    fread(end, sizeof(end), 1, file);
    fread(step, sizeof(step), 1, file);
    fread(&ndim, sizeof(ndim), 1, file);
    kc = malloc(sizeof(double) * ndim);
    fread(kc, sizeof(double) * ndim, 1, file);

    w = (end[0] - start[0]) / step[0] + 1;
    h = (end[1] - start[1]) / step[1] + 1;
    
    matrix = malloc(sizeof(float)*w*h);
    fread(matrix, sizeof(float), w*h, file);
    
    fclose(file);

    sprintf(out_filename, "%s.ppm", filename);
    file = fopen(out_filename, "wb");
    if(!file) {
      fprintf(stderr, "Unable to open %s\n", out_filename);
      free(matrix);
      continue;
    }
    
    ppm_w = w+50;
    ppm_h = h+20;
    fprintf(file, "P6\n%d %d\n%d\n", ppm_w, ppm_h, 255);

    ppm = malloc(ppm_w*ppm_h*3);
    memset(ppm, 0, ppm_w*ppm_h*3);

    for(y=0; y<h; y++) {
      z = y/(double)h;
      HSVtoRGB(z*240, 1.0, 1.0, color);
      for(x=0; x<20; x++) {
	ppm[((y+10)*ppm_w+(x+10))*3  ] = (int)(color[0] * 255 + 0.5);
	ppm[((y+10)*ppm_w+(x+10))*3+1] = (int)(color[1] * 255 + 0.5);
	ppm[((y+10)*ppm_w+(x+10))*3+2] = (int)(color[2] * 255 + 0.5);
      }
    }
    
    for(y=0; y<h; y++) {
      for(x=0; x<w; x++) {
	z = matrix[y*w+x];
	if(z < 0.0 || (1.0 < z && fitness_func != 1)) {
	  color[0] = 1;
	  color[1] = 1;
	  color[2] = 1;
	} else {
	  if(fitness_func == 1) {
	    if(z > 50) z = 50;
	    z = 1 - z/50;
	  }	    
	  HSVtoRGB((1.0-z)*240, 1.0, 1.0, color);
	}
	
	ppm[((y+10)*ppm_w+(x+40))*3  ] = (int)(color[0] * 255 + 0.5);
	ppm[((y+10)*ppm_w+(x+40))*3+1] = (int)(color[1] * 255 + 0.5);
	ppm[((y+10)*ppm_w+(x+40))*3+2] = (int)(color[2] * 255 + 0.5);
      }
    }

    {
      double kc1 = kc[param[0]];
      double kc2 = kc[param[1]];

      if(start[0] <= kc1 && kc1 <= end[0] &&
         start[1] <= kc2 && kc2 <= end[1]) {
	y = (int)(kc1 / step[0] + 0.5);
	x = (int)(kc2 / step[1] + 0.5);
     
        for(j=-5; j<=5; j++) {
  	  ppm[((y+10+j)*ppm_w+(x+40))*3  ] = 0;
	  ppm[((y+10+j)*ppm_w+(x+40))*3+1] = 0;
	  ppm[((y+10+j)*ppm_w+(x+40))*3+2] = 0;
	
	  ppm[((y+10)*ppm_w+(x+40+j))*3  ] = 0;
	  ppm[((y+10)*ppm_w+(x+40+j))*3+1] = 0;
	  ppm[((y+10)*ppm_w+(x+40+j))*3+2] = 0;
        }
      }
    }
    
    fwrite(ppm, ppm_w*ppm_h*3, 1, file);
    fclose(file);
    free(matrix);
    free(ppm);
  }
  
  return 0;
}
