# DTIPrep: DWI/DTI Quality Control Tool

## What is it?

DTIPrep performs a "Study-specific Protocol" based automatic pipeline for DWI/DTI quality control and preparation. This is both a GUI and command line tool. The configurable pipeline includes image/diffusion information check, padding/Cropping of data, slice-wise, interlace-wise and gradient-wise intensity and motion check, head motion and Eddy current artifact correction, and DTI computing.

## Build environment

You can find the necessary environment to build the software in Dockerfile.

## Dockerfile for developers

Use below command in source directory to build docker image (Currently having only CentOS7 image).

```
$ docker build . -t <image-name>
$ docker run --rm -it -v $PWD/../:/work -w /work <image-name> 
```

## Change Log:

#### [v1.3.0-beta](http://github.com/NIRALUser/DTIAtlasBuilder/tree/v1.3.0-beta) (07/10/2020)
- Superbuild script modified.
- Dockerfile added

## License

See License.txt

## More information

Find the tool on [NITRC](http://www.nitrc.org/projects/dtiprep/)

