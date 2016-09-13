const exec = require('child_process').exec;
const fs = require('fs');

const vulkanGlsl = 'C:/VulkanSDK/1.0.26.0/Bin32/glslangValidator.exe';

const walk = function (dir) {
  var results = []
  var list = fs.readdirSync(dir)
  list.forEach(function (file) {
    file = dir + '/' + file
    var stat = fs.statSync(file)
    if (stat && stat.isDirectory()) results = results.concat(walk(file))
    else results.push(file)
  })
  return results
}

const files = walk('./shaders');
const shaderExpression = /^.*\.(frag|vert|geo|geom|comp|vertex|fragment|tess)$/gi;

const shaderBin = './shaders/bin';
if (!fs.existsSync(shaderBin)) {
  fs.mkdirSync(shaderBin);
}

files.forEach(f=> {
  if (f.match(shaderExpression)) {
    var path = f.substring(0,f.lastIndexOf("/"));

    if (!fs.existsSync(path)) {
      fs.mkdirSync(path);
    }

    exec(`${vulkanGlsl} -V ${f} -o ${f}.spv`);
  }
});