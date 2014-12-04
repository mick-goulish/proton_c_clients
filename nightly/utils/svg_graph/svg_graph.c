#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>


typedef
struct datum_s
{
  double mean,
         sigma;
  char * date_stamp;
}
datum_t,
* datum_p;





double chart_width   = 1000,
       chart_height  = 800,
       grand_min     = 0.0,
       grand_max     = 20.0,
       y_range       = 20.0,
       y_scalar,
       x_axis_y_offset = 100;




double
transform_y ( double val )
{
  val -= grand_min;
  val *= y_scalar;
  val = chart_height - val;
  val -= x_axis_y_offset;

  return val;
}





void
svg ( datum_p data, int n_data, FILE * fp )
{
  int    day_label_font_size = 10;
  double mean_mark_height = 5;
  int    x_axis_length = 800;
  int    y_axis_length = 600;

  double origin_x = 100;
  int origin_y = x_axis_y_offset;

  double x_axis_y = chart_height - origin_y;

  int i;

  double day_label_x,
         day_label_y;

  double graph_line;


  /* The svg doc ------------------------------------------------- */
  fprintf ( fp, 
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n\n"
            "<svg \n"
            "  width=\"%lf\" \n"
            "  height=\"%lf\"\n"
            "  xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n"
            "  xmlns:cc=\"http://creativecommons.org/ns#\"\n"
            "  xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n"
            "  xmlns:svg=\"http://www.w3.org/2000/svg\"\n"
            "  xmlns=\"http://www.w3.org/2000/svg\"\n"
            "  version=\"1.1\"\n"
            ">\n\n\n",
            chart_width,
            chart_height
          );

  /* The chart label -----------------------------------------------*/
  fprintf ( fp, 
            "  <!-- chart label   -->\n"
            "  <text\n"
            "    x=\"200\"\n"
            "    y=\"30\"\n"
            "    xml:space=\"preserve\"\n"
            "    style=\"font-size:20px;font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;line-height:125%;letter-spacing:0px;word-spacing:0px;fill:#000000;fill-opacity:1;stroke:none;font-family:Serif;-inkscape-font-specification:Serif\">\n"
            "    <tspan\n"
            "      x=\"200\"\n"
            "      y=\"30\"\n"
            "    >\n"
            "Nightly Proton-C Engine Performance-Test Timings\n"
            "    </tspan>\n"
            "  </text>\n\n"
          );

  /* The chart sub-label -----------------------------------------------*/
  fprintf ( fp, 
            "  <!-- chart sub-label   -->\n"
            "  <text\n"
            "    x=\"200\"\n"
            "    y=\"70\"\n"
            "    xml:space=\"preserve\"\n"
            "    style=\"font-size:12px;font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;line-height:125%;letter-spacing:0px;word-spacing:0px;fill:#bbbbbb;fill-opacity:1;stroke:none;font-family:Serif;-inkscape-font-specification:Serif\">\n"
            "    <tspan\n"
            "      x=\"200\"\n"
            "      y=\"70\"\n"
            "    >\n"
            "Each day: 50 iterations of a 5 million message test.\n"
            "    </tspan>\n"
            "  </text>\n\n"
          );

  /* The chart sub-label -----------------------------------------------*/
  fprintf ( fp, 
            "  <!-- chart sub-label   -->\n"
            "  <text\n"
            "    x=\"200\"\n"
            "    y=\"90\"\n"
            "    xml:space=\"preserve\"\n"
            "    style=\"font-size:11px;font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;line-height:125%;letter-spacing:0px;word-spacing:0px;fill:#bbbbbb;fill-opacity:1;stroke:none;font-family:Serif;-inkscape-font-specification:Serif\">\n"
            "    <tspan\n"
            "      x=\"200\"\n"
            "      y=\"90\"\n"
            "    >\n"
            "Display shows mean and +- 1 sigma.\n"
            "    </tspan>\n"
            "  </text>\n\n"
          );

  /* The chart sub-sub-label -----------------------------------------------*/
  fprintf ( fp, 
            "  <!-- chart sub-sub-label   -->\n"
            "  <text\n"
            "    x=\"200\"\n"
            "    y=\"110\"\n"
            "    xml:space=\"preserve\"\n"
            "    style=\"font-size:10px;font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;line-height:125%;letter-spacing:0px;word-spacing:0px;fill:#bbbbbb;fill-opacity:1;stroke:none;font-family:Serif;-inkscape-font-specification:Serif\">\n"
            "    <tspan\n"
            "      x=\"200\"\n"
            "      y=\"110\"\n"
            "    >\n"
            "Zoom in on mean-marks to see exact timings.\n"
            "    </tspan>\n"
            "  </text>\n\n"
          );

  /* The x-axis ----------------------------------------------------- */
  fprintf ( fp, 
            "  <!-- x axis   -->\n"
            "  <rect\n"
            "    width=\"%d\"\n"
            "    height=\"2\"\n"
            "    x=\"%lf\"\n"
            "    y=\"%lf\"\n"
            "    style=\"fill:#000000;fill-opacity:1;stroke:none\" \n"
            "  />\n\n",
            x_axis_length,
            origin_x,
            x_axis_y
          );

  /* The x-axis label -----------------------------------------------*/
  fprintf ( fp, 
            "  <!-- x axis label   -->\n"
            "  <text\n"
            "    x=\"%lf\"\n"
            "    y=\"%lf\"\n"
            "    xml:space=\"preserve\"\n"
            "    style=\"font-size:18px;font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;line-height:125%;letter-spacing:0px;word-spacing:0px;fill:#000000;fill-opacity:1;stroke:none;font-family:Serif;-inkscape-font-specification:Serif\">\n"
            "    <tspan\n"
            "      x=\"%lf\"\n"
            "      y=\"%lf\"\n"
            "    >\n"
            "      dates\n"
            "    </tspan>\n"
            "  </text>\n\n",
            origin_x + 300,
            x_axis_y + 100,
            origin_x + 300,
            x_axis_y + 100
          );


  /* The y-axis ---------------------------------------------------- */
  fprintf ( fp,
            "  <!-- y axis  -->\n"
            "  <rect\n"
            "    style=\"fill:#000000;fill-opacity:1;stroke:none\"\n"
            "    width=\"2\"\n"
            "    height=\"%d\"\n"
            "    x=\"%lf\"\n"
            "    y=\"%d\"\n"
            "  />\n\n",
            y_axis_length,
            origin_x,
            origin_y
          );

  /* y-axis label ---------------------------------------------------- */
  fprintf ( fp,
            "  <!-- y axis label   -->\n"
            "  <text\n"
            "    x=\"%lf\"\n"
            "    y=\"%d\"\n"
            "    transform=\"matrix(0,-1,1,0,0,0)\"\n"
            "    xml:space=\"preserve\"\n"
            "    style=\"font-size:18px;font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;line-height:125%;letter-spacing:0px;word-spacing:0px;fill:#000000;fill-opacity:1;stroke:none;font-family:Serif;-inkscape-font-specification:Serif\">\n"
            "    <tspan\n"
            "      x=\"%d\"\n"
            "      y=\"%d\"\n"
            "    >\n"
            "    seconds\n"
            "    </tspan>\n"
            "  </text>\n\n",
            origin_x - 10,
            100,
            -500,
            40
          );

  /*-------------------------------------
  grand_min = 999999999;
  grand_max = -999999999;

    Calculate grand min and max.
  for ( i = 0; i < n_data; ++ i )
  {
    double val;
    datum_p d = data + i;
    
    val = d->mean + d->sigma;
    if ( val > grand_max )
      grand_max = val;

    val = d->mean - d->sigma;
    if ( val < grand_min )
      grand_min = val;
  }
  y_range = grand_max - grand_min;
  -------------------------------------*/

  y_scalar = (chart_height - x_axis_y_offset) / y_range;
  y_scalar *= 0.8;


  /*
    draw horizontal lines from grand_min + 1 to grand_max.
    These grand extremes are chose by hand, to make the graph look reasonable.
  */

  for ( graph_line = grand_min; graph_line <= grand_max;  graph_line += 1 )
  {
    double line_y  = transform_y ( graph_line );
    double label_y = line_y + 4.0;
    double label_x = 40.0;

    fprintf ( fp, 
              "  <!-- graph line   -->\n"
              "  <rect\n"
              "    width=\"%d\"\n"
              "    height=\"1\"\n"
              "    x=\"%lf\"\n"
              "    y=\"%lf\"\n"
              "    style=\"fill:#dddddd;fill-opacity:1;stroke:none\" \n"
              "  />\n\n",
              x_axis_length,
              origin_x,
              line_y
            );

    /* graph-line label -----------------------------------------------*/
    fprintf ( fp, 
              "  <!-- graph line label  -->\n"
              "  <text\n"
              "    x=\"%lf\"\n"
              "    y=\"%lf\"\n"
              "    xml:space=\"preserve\"\n"
              "    style=\"font-size:12px;font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;line-height:125%;letter-spacing:0px;word-spacing:0px;fill:#888888;fill-opacity:1;stroke:none;font-family:Serif;-inkscape-font-specification:Serif\">\n"
              "    <tspan\n"
              "      x=\"%lf\"\n"
              "      y=\"%lf\"\n"
              "    >\n"
              "      %.0lf\n"
              "    </tspan>\n"
              "  </text>\n\n",
              label_x,
              label_y,
              label_x,
              label_y,
              graph_line
            );

  }

  /*===============================================================
    Mean marks and sigma lines for each day's measurements.
  ===============================================================*/
  for ( i = 0; i < n_data; ++ i )
  {
    datum_p d = data + i;
    int x_scale = 25;
    double mark_x = origin_x + x_scale * (i + 1);
    double mark_y      = transform_y ( d->mean );
    double line_top    = transform_y ( d->mean + d->sigma );
    double line_bottom = transform_y ( d->mean - d->sigma );
    double line_height = line_bottom - line_top;
    char   mark_label[200];
    int mean_mark_width = 15;
    

    /* line top  ------------------------------------------------------- */
    fprintf ( fp, 
              "  <!-- line top    -->\n"
              "  <rect\n"
              "    style=\"fill:#330000;fill-opacity:1;stroke:none\"\n"
              "    width=\"%d\"\n"
              "    height=\"1\"\n"
              "    x=\"%lf\"\n"
              "    y=\"%lf\"\n"
              "  />\n\n",
              mean_mark_width,
              mark_x,
              line_top
            );

    /* line bottom  ------------------------------------------------------- */
    fprintf ( fp, 
              "  <!-- line bottom    -->\n"
              "  <rect\n"
              "    style=\"fill:#330000;fill-opacity:1;stroke:none\"\n"
              "    width=\"%d\"\n"
              "    height=\"1\"\n"
              "    x=\"%lf\"\n"
              "    y=\"%lf\"\n"
              "  />\n\n",
              mean_mark_width,
              mark_x,
              line_bottom
            );

    /* sigma line ------------------------------------------------------ */
    fprintf ( fp, 
              "  <!-- sigma line   -->\n"
              "  <rect\n"
              "style=\"fill:#784421;fill-opacity:0.37837841999999999;stroke:none\"\n"
              "width=\"2.5\"\n"
              "height=\"%lf\"\n"
              "x=\"%lf\"\n"
              "y=\"%lf\"\n"
              "/>\n",
              line_height,
              mark_x + 7,
              line_top
            );

    /* mean mark ------------------------------------------------------- */
    fprintf ( fp, 
              "  <!-- mean mark   -->\n"
              "  <rect\n"
              "    style=\"fill:#ff0000;fill-opacity:1;stroke:none\"\n"
              "    width=\"%d\"\n"
              "    height=\"%lf\"\n"
              "    x=\"%lf\"\n"
              "    y=\"%lf\"\n"
              "  />\n\n",
              mean_mark_width,
              mean_mark_height,
              mark_x,
              mark_y - mean_mark_height / 2
            );

     sprintf ( mark_label, "%.2lf", d->mean );


    /* The mean label -----------------------------------------------*/
    fprintf ( fp, 
              "  <!-- mean label   -->\n"
              "  <text\n"
              "    x=\"%lf\"\n"
              "    y=\"%lf\"\n"
              "    xml:space=\"preserve\"\n"
              "    style=\"font-size:5px;font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;line-height:125%;letter-spacing:0px;word-spacing:0px;fill:#000000;fill-opacity:1;stroke:none;font-family:Serif;-inkscape-font-specification:Serif\">\n"
              "    <tspan\n"
              "      x=\"%lf\"\n"
              "      y=\"%lf\"\n"
              "    >\n"
              "      %s\n"
              "    </tspan>\n"
              "  </text>\n\n",
              mark_x - 12,
              mark_y + mean_mark_height / 2,
              mark_x - 12,
              mark_y + mean_mark_height / 2,
              mark_label
            );

    day_label_x = mark_x + mean_mark_width  / 2 + day_label_font_size / 2;
    day_label_y = chart_height - 20;

    fprintf ( fp,
              "  <!-- day label   -->\n"
              "  <text \n"
              "    x=\"%lf\" \n"
              "    y=\"%lf\" \n"
              "    transform=\"rotate(-90 %lf,%lf)\"\n"
              "    style=\"font-size:%dpx;font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;line-height:125%;letter-spacing:0px;word-spacing:0px;fill:#000000;fill-opacity:1;stroke:none;font-family:Serif;-inkscape-font-specification:Serif\"\n"
              "  >\n"
              "    %s\n"
              "  </text>\n",
              day_label_x,
              day_label_y,
              day_label_font_size,
              day_label_x,
              day_label_y,
              d->date_stamp 
            );

  }


  fprintf ( fp, "</svg>\n\n\n" );
}




#define MAX_FILES 10000
int n_files = 0;
char * file_names [ MAX_FILES ];





void
print_file_names ( )
{
  int i;

  fprintf ( stdout, "-------------------------------------\n" );
  for ( i = 0; i < n_files; ++ i )
  {
    fprintf ( stdout, "%s\n", file_names [ i ] );
  }
}





void
get_file_names ( char const * dir_name )
{
  DIR * dir;
  struct dirent * file;
  char full_name[2000];

  dir = opendir (dir_name );
  while ( file = readdir ( dir ) )
  {
    if ( file->d_type == DT_REG )
    {
      sprintf ( full_name, "%s/%s", dir_name, file->d_name );
      file_names [ n_files ] = strdup ( full_name );
      ++ n_files;
    }
  }
  closedir ( dir );
}





void
sort_file_names ( )
{
  int i;
  int sorted = 0;

  while ( ! sorted )
  {
    sorted = 1;

    for ( i = 0; i < n_files - 1; ++ i )
    {
      if ( 0 < strcmp ( file_names[i], file_names[i+1] ) )
      {
        char * temp = file_names[i];
        file_names[i] = file_names[i+1];
        file_names[i+1] = temp;
        sorted = 0;
      }
    }
  }
}





#define MAX_DATA 10000


int
main ( int argc, char ** argv )
{
  int i;
  char const * dir_name         = (argc > 1) ? argv[1] : ".";
  char const * output_file_name = (argc > 2) ? argv[2] : "./nightly.svg";
  FILE * fp,
       * output_fp;
  int n_data = 0;
  datum_t data [ MAX_DATA ];
  datum_p d;

  get_file_names ( dir_name );
  sort_file_names ( );

  for ( i = 0, d = data; i < n_files; ++ i, ++ d )
  {
    fprintf ( stdout, "reading file: |%s|\n", file_names[i] );
    fp = fopen ( file_names[i], "r" );
    fscanf ( fp, "%lf%lf", & (d->mean), & (d->sigma) );
    d->date_stamp = strdup ( strrchr ( file_names[i], '/' ) + 1 );
    fclose ( fp );
    ++ n_data;
  }

  output_fp = fopen ( output_file_name, "w" );
  svg ( data, n_data, output_fp );
  fclose ( output_fp );
}





