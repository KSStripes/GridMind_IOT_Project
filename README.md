# GridMind

GridMind is a student IoT project that uses a physical serious game to explain
simple data-centre workload decisions.

The project uses two ESP8266 nodes:

- **Node A** represents facility conditions such as available compute capacity,
  electricity and temperature.
- **Node B** presents fictional contracts and lets the learner choose Run,
  Wait or Cancel using physical buttons.

The nodes will communicate through local Wi-Fi. Each node will provide a small
browser dashboard and REST/HTTP JSON responses. LEDs give immediate physical
feedback, while fictional contract values and penalties show the result of a
decision.

Node B is implemented and working. Node A and the connection between the two
nodes are the next development stages.

GridMind is an educational prototype, not a real data-centre controller or
billing system. All facility conditions, contracts and monetary values are
synthetic.
