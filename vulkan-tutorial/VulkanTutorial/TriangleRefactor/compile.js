const exec = require('child_process').exec;
const fs = require('fs');

const vulkanGlsl = '%VULKAN_SDK%/Bin32/glslangValidator.exe';
console.log("Building");
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

files.forEach(f=> {
  if (f.match(shaderExpression)) {
    var path = f.substring(0, f.lastIndexOf("/"));
    if (!fs.existsSync(path)) fs.mkdirSync(path);
    var command = `${vulkanGlsl} -V ${f} -o ${f}.spv`;

    console.log("Running the following command: " + command);

    exec(command, (error, stdout, stderr) => {
      if (error) console.error(`exec ${error}`);
      if (stdout) console.log(`stdout: ${stdout.trim()}`);
      if(stderr) console.log(`stderr: ${stderr.trim()}`);
    });
  }
});