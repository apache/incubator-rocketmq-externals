package org.apache.rocketmq.flink.common.serialization.json;

import org.apache.flink.formats.json.TimestampFormat;
import org.apache.flink.shaded.jackson2.com.fasterxml.jackson.databind.JsonNode;
import org.apache.flink.shaded.jackson2.com.fasterxml.jackson.databind.ObjectMapper;
import org.apache.flink.table.data.RowData;
import org.apache.flink.table.runtime.typeutils.RowDataTypeInfo;
import org.apache.flink.table.types.logical.RowType;
import org.apache.flink.util.Collector;
import org.apache.rocketmq.common.message.Message;
import org.junit.Test;

import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;

import static org.apache.flink.table.api.DataTypes.DOUBLE;
import static org.apache.flink.table.api.DataTypes.FIELD;
import static org.apache.flink.table.api.DataTypes.INT;
import static org.apache.flink.table.api.DataTypes.ROW;
import static org.apache.flink.table.api.DataTypes.STRING;
import static org.junit.Assert.assertEquals;

/**
 * @Author: gaobo07
 * @Date: 2020/10/12 11:59 上午
 */
public class RmqJsonSerDerTest {

    private static final RowType SCHEMA = (RowType) ROW(
            FIELD("id", INT().notNull()),
            FIELD("name", STRING()),
            FIELD("description", STRING()),
            FIELD("weight", DOUBLE())
    ).getLogicalType();

    private static ObjectMapper objectMapper = new ObjectMapper();

    @Test
    public void testSerializationDeserialization() throws Exception {
        List<String> lines = readLines("RmqJson-data.txt");
        RmqJsonDeserializer deserializationSchema = new RmqJsonDeserializer(
                SCHEMA,
                new RowDataTypeInfo(SCHEMA),
                false,
                TimestampFormat.ISO_8601);

        SimpleCollector collector = new SimpleCollector();
        for (String line : lines) {
            deserializationSchema.deserialize(line.getBytes(StandardCharsets.UTF_8), collector);
        }

        List<String> expected = Arrays.asList(
                "+I(101,scooter,Small 2-wheel scooter,3.14)",
                "+I(102,car battery,12V car battery,8.1)",
                "+I(103,12-pack drill bits,12-pack of drill bits with sizes ranging from #40 to #3,0.8)",
                "+I(104,hammer,12oz carpenter's hammer,0.75)",
                "+I(105,hammer,14oz carpenter's hammer,0.875)",
                "+I(106,hammer,16oz carpenter's hammer,1.0)",
                "+I(107,rocks,box of assorted rocks,5.3)",
                "+I(108,jacket,water resistent black wind breaker,0.1)",
                "+I(109,spare tire,24 inch spare tire,22.2)",
                "+I(110,jacket,water resistent white wind breaker,0.2)",
                "+I(111,scooter,Big 2-wheel scooter ,5.18)"
        );
        List<String> actual = collector.list.stream()
                .map(Object::toString)
                .collect(Collectors.toList());
        assertEquals(expected, actual);

        Integer sinkKeyPos = 0;

        RmqJsonSerializer serializationSchema = new RmqJsonSerializer(
                SCHEMA,
                TimestampFormat.SQL,
                sinkKeyPos);
        serializationSchema.open(null);
        List<String> result = new ArrayList<>();
        for (RowData rowData : collector.list) {
            result.add(serializationSchema.serialize(rowData).toString());
        }

        List<String> tmpBodyList = Arrays.asList(
                "{\"id\":101,\"name\":\"scooter\",\"description\":\"Small 2-wheel scooter\",\"weight\":3.14}",
                "{\"id\":102,\"name\":\"car battery\",\"description\":\"12V car battery\",\"weight\":8.1}",
                "{\"id\":103,\"name\":\"12-pack drill bits\",\"description\":\"12-pack of drill bits with sizes ranging from #40 to #3\",\"weight\":0.8}",
                "{\"id\":104,\"name\":\"hammer\",\"description\":\"12oz carpenter's hammer\",\"weight\":0.75}",
                "{\"id\":105,\"name\":\"hammer\",\"description\":\"14oz carpenter's hammer\",\"weight\":0.875}",
                "{\"id\":106,\"name\":\"hammer\",\"description\":\"16oz carpenter's hammer\",\"weight\":1.0}",
                "{\"id\":107,\"name\":\"rocks\",\"description\":\"box of assorted rocks\",\"weight\":5.3}",
                "{\"id\":108,\"name\":\"jacket\",\"description\":\"water resistent black wind breaker\",\"weight\":0.1}",
                "{\"id\":109,\"name\":\"spare tire\",\"description\":\"24 inch spare tire\",\"weight\":22.2}",
                "{\"id\":110,\"name\":\"jacket\",\"description\":\"water resistent white wind breaker\",\"weight\":0.2}",
                "{\"id\":111,\"name\":\"scooter\",\"description\":\"Big 2-wheel scooter \",\"weight\":5.18}"
        );

        List<String> expectedResult = new ArrayList<>();
        for(String body : tmpBodyList){
            JsonNode jsonNode = objectMapper.readTree(body);
            expectedResult.add(new Message("", "", jsonNode.get( SCHEMA.getFields().get(sinkKeyPos).getName()).asText(), body.getBytes() ).toString());
        }

        assertEquals(expectedResult, result);
    }

    // --------------------------------------------------------------------------------------------
    // Utilities
    // --------------------------------------------------------------------------------------------

    private static List<String> readLines(String resource) throws IOException {
        final URL url = RmqJsonSerializer.class.getClassLoader().getResource(resource);
        assert url != null;
        Path path = new File(url.getFile()).toPath();
        return Files.readAllLines(path);
    }

    private static class SimpleCollector implements Collector<RowData> {

        private List<RowData> list = new ArrayList<>();

        @Override
        public void collect(RowData record) {
            list.add(record);
        }

        @Override
        public void close() {
            // do nothing
        }
    }

}
