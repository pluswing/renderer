#version 120

varying vec3 n;

void main(void) {
  vec3 l = normalize(gl_LightSource[0].position.xyz);
  float intensity = 0.2 + max(dot(l, normalize(n)), 0.0);

  if (intensity > 0.95) {
    intensity = 1;
  } else if (intensity > 0.5) {
    intensity = 0.6;
  } else if (intensity > 0.25) {
    intensity = 0.4;
  } else {
    intensity = 0.2;
  }

  gl_FragColor = gl_Color * intensity;
  return;
}
